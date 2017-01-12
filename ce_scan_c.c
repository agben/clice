//--------------------------------------------------------------
//
// Scan C source files and update the clice database
//
//	usage:	ce_scan_c <source.c>
//		where	source.c	= source filename or library name i.e. library.h
//			--version	= version, copyright and licence details
//			--help		= basic help
//
// #TODO - could also be used to check for edit permissions or other warnings?
//
//	GNU GPLv3+ licence	clice - The command-line coding ecosystem by Andrew Bennington 2017 <www.benningtons.net>
//
//--------------------------------------------------------------

#include <ctype.h>		// for toupper
#include <fcntl.h>		// file access functions
#include <getopt.h>		// for getopt_long - argument parsing
#include <string.h>		// for strcopy

#include <fa_def.h>		// file actions
#include <ce_main.h>	// CE database
#include <ut_date.h>	// date and time utilities
#include <ut_error.h>	// for debug and checks

#define	BUFF_S0 200

int main(int argc, char **argv)
  {
	void ce_scan_c_update(void);
	FILE *fp;
	char sBuff[BUFF_S0];		// input buffer for init file reads
	int i, j, k;
	char *cp;

	spCE = (struct CE_FIELDS*) &CE;
	spCEL = (struct CEL_FIELDS*) &CEL;

								// valid arguments: name, has_arg(yes=1, no=0, opt=2), flag, val
	static struct option long_options[] = {
		{"help",	0,	0,		0},		// 0	Keep this order for parsing after getopt_long
		{"version",	0,	0,		0},		// 1
		{NULL,		0,	NULL,	0}
	};

	int option_index = 0;

	while ((i=getopt_long(	argc,				//number of arguments
							argv,				//argument values - an array of pointers to each argument
							"",					//permitted short arguments  i.e. -v (none permitted)
							long_options,		//permitted long arguments   i.e. --version
							&option_index)) != -1)
	 {
		if (i == '?' || i != 0)					// invalid arg or arg qualifier so abort
			return -1;
		else if (option_index == 0)				// --help
		 {
			ce_help();
			return 0;
		 }
		else if (option_index == 1)				// --version
		 {
			ce_version();
			return 0;
		 }
	 }


	ut_check(optind == argc-1,					// still an argument remaining? the program to scan
		"Usage = ce_scan_c <source.c>");

	strncpy(	CE.sSource,						// Store source file name
				argv[optind],
				CE_SOURCE_S0);

	ut_check(cef_main(FA_INIT+FA_OPEN, 0) == 0,	// Initialise libgxtfa and open clice db
			"Failed to open clice db");

	ut_check(getcwd(CE.sDir,
				sizeof(CE.sDir)), "getcwd");	// get current working directory
	strncpy(CE.sProject,
			&CE.sDir[strlen(CE.sDir)+1-CE_PROJECT_S0],
			CE_PROJECT_S0);						// get project code

	CE.sDesc[0]='\0';							// Mark description field as empty

	fp = fopen(CE.sSource, "r");				// Open file as read-only
	ut_check(fp != NULL, "open source file");	// jumps to error: if not ok

	while (fgets(sBuff, BUFF_S0, fp) != NULL &&
			CE.sDesc[0] == '\0')				// Stop once we have a description - remove when further checks added
	  {
		ut_debug("in: %s", sBuff);

		if (sBuff[0] == '/' && sBuff[1] == '/')	// look for comment lines	#TODO check for /* comments */
		  {
			if (CE.sDesc[0] == '\0')			// Still not found a description
			  {
				j=0;							// check on the number of non-alphabetics used
				k=0;							// output counter
				i=2;							// start reading commented line after the comment markers

				int iSpace=0;					// flag to exclude leading or multiple spaces in a description
				while (sBuff[i] != '\n' && k < (CE_DESC_S0 - 1))	// unpack description
				  {
					if (sBuff[i] > ' ' ||
						(iSpace == 1 && sBuff[i] == ' '))
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
	sprintf(sBuff, "%.*s.ce", i, CE.sSource);

	CE.cLang=toupper(CE.sSource[i+1]);			// #TODO - validate
	CE.iStatus=0;
	CE.iCDate=CE.iMDate;
	CE.iCTime=CE.iMTime;
	CE.iSize=0;

	fp = fopen(sBuff, "r");						// Open ctags file as read-only
	ut_check(fp != NULL, "open ctags file");	// jumps to error: if not ok

	while (fgets(sBuff, BUFF_S0, fp) != NULL)
	  {
		ut_debug("in: %s", sBuff);

		for (i = 0; i < BUFF_S0 && sBuff[i] != ' '; i++);	// skip past item name and
		for ( ; i < BUFF_S0 && sBuff[i] == ' '; i++);		// skip past following spaces to find item type

		if (memcmp(&sBuff[i], "function", 8) == 0)			// is the item a function definition?
		  {
			if (memcmp(sBuff, "main", 4) == 0)				// replace 'main' modules with the source filename
				for (j = 0; j < CE_NAME_S0 &&
						CE.sSource[j] != '\0' &&
						CE.sSource[j] != '.'; j++)
					CE.sName[j]=CE.sSource[j];				// #TODO warn and ignore names that are too long
			else
				for (j=0; j < (CE_NAME_S0-1) &&
									sBuff[j] != ' '; j++)	// otherwise copy the item name
					CE.sName[j]=sBuff[j];
			for ( ; j < CE_NAME_S0; j++) CE.sName[j]='\0';  // null fill remainder of string

			i+=8;											// skip function
			for (; i < BUFF_S0 && sBuff[i] == ' '; i++);	// skip past following spaces
			for (; i < BUFF_S0 && sBuff[i] != ' '; i++);	// skip past line number
			for (; i < BUFF_S0 && sBuff[i] == ' '; i++);	// skip past following spaces
			for (; i < BUFF_S0 && sBuff[i] != ' '; i++);	// skip past source file name
			for (; i < BUFF_S0 && sBuff[i] == ' '; i++);	// skip past following spaces
			for (j=0; j < (CE_CODE_LINE_S0-1) &&
									sBuff[i] != '\n'; j++)	// Extract function definition
				CE.sCode[j]=sBuff[i++];
			for (; j < CE_CODE_LINE_S0; j++) CE.sCode[j]='\0';  // null fill remainder of string

			ce_scan_c_update();								// update clice db
		  }
	  }

	if (CE.cLang == 'H')								// need to update library modules
	  {
		for (i = 0; i < CE_NAME_S0 &&
					CE.sSource[i] != '\0' &&
					CE.sSource[i] != '.'; i++)
			CE.sName[i]=CE.sSource[i];
		for ( ; i < CE_NAME_S0; i++) CE.sName[i]='\0';  // null fill remainder of string

		ce_scan_c_update();								// update clice db
	  }


error:
	if (fp != NULL) fclose(fp);
	cef_main(FA_CLOSE, 0);
	return 0;
  };


//--------------------------------------------------------------
//
// Update or create a clice db entry for the details held in the CE struct
//
//--------------------------------------------------------------

void ce_scan_c_update(void)
  {
	int ios;


	CE.bmField=CEF_ID_B0;							// only need to read ID to confirm if this module exists in CE
	CEL.bmField=0;
	if (cef_main(FA_READ+FA_KEY1+FA_STEP, 0) == FA_NODATA_IV0)	// module not found so create a new CE module
	  {
		if (CE.cLang == 'H')
		  {
			printf("CE: New library module added - %s\n", CE.sName);
			CE.iType=CE_LIB_T0;
		  }
		else
		  {
			printf("CE: New program module added - %s\n", CE.sName);
			CE.iType=CE_PRG_T0;
		  }

		CE.bmField=FA_ALL_COLS_B0;				// Going to insert all new fields
		ios=cef_main(FA_WRITE, 0);				// Write a new CE module to db
		ut_check(ios == FA_OK_IV0, "write %d", ios);
	  }
	else										// exists so update last modified date/time
	  {
		CE.bmField=CEF_LAST_MOD_B0+CEF_DESC_B0;	// and description (in case changed)
		ios=cef_main(FA_UPDATE+FA_KEY0, 0);		// update CE module in db
		ut_check(ios == FA_OK_IV0, "rewrite %d", ios);
	  }
error:
	return;
  };
