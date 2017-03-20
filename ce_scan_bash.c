//--------------------------------------------------------------
//
// Scan bash scripts and update the clice database
//
//	usage:	ce_scan_bash <script.sh>
//		where	script.sh	= bash script filename
//			--version	= version, copyright and licence details
//			--help		= basic help
//
// #TODO - could also be used to check for edit permissions or other warnings?
//
//	GNU GPLv3+ licence	clice - The command-line coding ecosystem by Andrew Bennington 2017 <www.benningtons.net>
//
//--------------------------------------------------------------

#include <fcntl.h>		// file access functions
#include <string.h>		// for strcopy

#include <fa_def.h>		// file actions
#include <ce_main.h>	// CE database
#include <ut_date.h>	// date and time utilities
#include <ut_error.h>	// for debug and checks

#define	BUFF_S0 200

int main(int argc, char **argv)
  {
	int ios=0;

	FILE *fp = 0;
	char sBuff[BUFF_S0];		// input buffer for init file reads
	int i, j, k;
	char *cp;

	spCE = (struct CE_FIELDS*) &CE;
	spCEL = (struct CEL_FIELDS*) &CEL;


	i=ce_args(argc, argv);			// parse command arguments i.e. ce_scan_bash --version
	if (i < 0) goto error;
	ut_check(i == argc-1,			// still an argument remaining? the program to scan
		"Usage = ce_scan_bash <script.sh>");

	strncpy(	CE.sSource,			// Store source file name
				argv[i],
				CE_SOURCE_S0);

	CE.cLang='S';

	ut_check(cef_main(FA_INIT+FA_OPEN, 0) == 0,	// Initialise libgxtfa and open clice db
			"Failed to open clice db");

	ut_check(getcwd(CE.sDir, sizeof(CE.sDir)), "getcwd");	// get current working directory

	strncpy(CE.sProject,
			&CE.sDir[strlen(CE.sDir)+1-CE_PROJECT_S0],
			CE_PROJECT_S0);						// get project code

	CE.sDesc[0]='\0';							// Mark description field as empty

	fp = fopen(CE.sSource, "r");				// Open file as read-only
	ut_check(fp != NULL, "open source file %s",
								CE.sSource);	// jumps to error: if not ok

	while (fgets(sBuff, BUFF_S0, fp) != NULL &&
			CE.sDesc[0] == '\0')				// Stop once we have a description - remove when further checks added
	  {
		ut_debug("in: %s", sBuff);

		if (sBuff[0] == '#')					// look for # comment lines
		  {
			if (CE.sDesc[0] == '\0')			// Still not found a description
			  {
				j=0;							// check on the number of non-alphabetics used
				k=0;							// output counter
				i=1;							// start reading commented line after the comment marker

				int iSpace=0;					// flag to exclude leading or multiple spaces in a description
				while (sBuff[i] != '\n' &&
						k < (CE_DESC_S0 - 1))	// unpack description
				  {
					if (sBuff[i] != '\'' &&
						sBuff[i] != '\"' &&
						sBuff[i] != '`' &&		// to prevent an accidental SQL injection, ignore these!
						(sBuff[i] > ' ' ||
						(iSpace == 1 && sBuff[i] == ' ')))
					  {
						CE.sDesc[k++]=sBuff[i];
						if (sBuff[i] == ' ')
							iSpace=0;			// had a space so don't allow any more
						else
							iSpace=1;			// had a character so allow a space

						if ( sBuff[i] < 'A' || sBuff[i] > 'z' ||
							(sBuff[i] > 'Z' && sBuff[i] < 'a')) j++;
					  }
					i++;
				  }

				if ((k-j) < 10) k=0;			// Not enough alphabetic characters to be a description
				CE.sDesc[k]='\0';
			  }
			else
				continue;						// Have a description so ignore commented lines
		  }
	  }

	fclose(fp);									// finished with source code file
	fp=NULL;

	ut_date_now();								// get current date and time
	CE.iMDate=gxt_iDate[0];
	CE.iMTime=gxt_iTime[0];

	cp=strrchr(CE.sSource,'.');					// Find last dot in the passed program name before the file extension
	ut_check( cp != NULL,
			"No valid file extension");
	i=cp-CE.sSource;							// size of program name minus file extension
	if (i > CE_NAME_S0) i=CE_NAME_S0;			// #TODO warn about truncated filename
	sprintf(CE.sName, "%.*s", i, CE.sSource);

	CE.bmField=CEF_ID_B0;						// only need to read ID to confirm if this program exists in CE
	CEL.bmField=0;
	if (cef_main(FA_READ+FA_KEY1+FA_STEP, 0) == FA_NODATA_IV0)	// module not found so create a new CE module
	 {
		printf("CE: New shell script module added - %s\n", CE.sName);
		CE.iType=CE_PROG_T0;

		CE.cMain=' ';								// n/a for scripts
		CE.cIgnore=' ';
		CE.cLibrary=' ';
		CE.cSpare=' ';
		CE.iCDate=CE.iMDate;
		CE.iCTime=CE.iMTime;
		CE.iSize=0;

		CE.bmField=FA_ALL_COLS_B0;				// Going to insert all new fields
		ios=cef_main(FA_WRITE, 0);				// Write a new CE module to db
		ut_check(ios == FA_OK_IV0, "write %d", ios);
	 }
	else										// exits so update last modified date/time
	 {
		CE.bmField=CEF_LAST_MOD_B0+CEF_DESC_B0;	// only need to update last modified date & time
		ios=cef_main(FA_UPDATE+FA_KEY0, 0);		// update CE module in db
		ut_check(ios == FA_OK_IV0, "rewrite %d", ios);
	 }

error:
	if (fp != NULL) fclose(fp);
	cef_main(FA_CLOSE, 0);
	return ios;
  }
