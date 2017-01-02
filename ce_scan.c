//--------------------------------------------------------------
//
// Basic clice updater based on passed parameters - doesn't scan source or object files (yet)
//
//	usage:	ce_scan <prog> --language ?
//		where	prog		= program name
//			--language <?>	= set program language [mandatory]
//			--version	= version, copyright and licence details
//			--help		= basic help
//
// #TODO - could also be used to check for edit permissions or other warnings?
//
//	GNU GPLv3+ licence	clice - The command-line coding ecosystem by Andrew Bennington 2015 <www.benningtons.net>
//
//--------------------------------------------------------------

#include <fcntl.h>		// file access functions
#include <getopt.h>		// for getopt_long - argument parsing
#include <stdio.h>
#include <stdlib.h>
#include <string.h>		// for strcopy

#include <fa_def.h>		// file actions
#include <ce_main.h>	// CE database
#include <ut_date.h>	// date and time utilities
#include <ut_error.h>	// for debug and checks

#define	BUFF_S0 200

int main(int argc, char **argv)
  {
	int ios=0;

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
		{"language",1,	0,		0},		// 2
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
		 {
			return -1;
		 }
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
		else if (option_index == 2)				// --language
		 {
			spCE->cLang=optarg[0];				// #TODO - validate
		 }
	 }

	ut_check(optind == argc-1,					// still an argument remaining? the program to scan
			"Usage = ce_scan <program> --language c");

	strncpy(	spCE->sSource,					// Store source file name
				argv[optind],
				CE_SOURCE_S0);

	cp=strrchr(spCE->sSource,'.');				// Find last dot in the passed program name before the file extension
	ut_check( cp != NULL,
			"No valid file extension");
	i=cp-spCE->sSource;							// size of program name minus file extension
//	i=cp-&argv[optind][0];						// size of program name minus file extension
	if (i > CE_NAME_S0) i=CE_NAME_S0;			// #TODO warn about truncated filename
	strncpy(	spCE->sName,					// copy string and ensure a trailing null
				spCE->sSource,
				i);								// Extract program name but exclude the extension

	ut_check(cef_main(FA_INIT+FA_OPEN, 0) == 0,	// Initialise libgxtfa and open clice db
			"Failed to open clice db");

	ut_check(getcwd(CE.sDir, sizeof(CE.sDir)), "getcwd");	// get current working directory

	ut_debug("Program to scan= %s",spCE->sName);
	ut_debug("Current directory= %s",spCE->sDir);
	ut_debug("language = %c",spCE->cLang);

	spCE->sDesc[0]=0;							// Mark description field as empty

	fp = fopen(argv[optind], "r");				// Open file as read-only
	ut_check(fp != NULL, "open source file");	// jumps to error: if not ok

	while (fgets(sBuff, BUFF_S0, fp) != NULL &&
			spCE->sDesc[0] == 0)				// Stop once we have a description - remove when further checks added
	  {
		ut_debug("in: %s", sBuff);

		if (((spCE->cLang == 'C' || spCE->cLang == 'H') &&	// C code so look for // comment lines
			sBuff[0] == '/' && sBuff[1] == '/') ||
			(spCE->cLang == 'S' && sBuff[0] == '#'))		// Shell script so look for # comment lines
		  {
			if (spCE->sDesc[0] == 0)			// Still not found a description
			  {
				j=0;							// check on the number of non-alphabetics used
				k=0;							// output counter
				if (spCE->cLang == '#')
					i=1;						// start reading commented line after the comment marker
				else
					i=2;						// or markers for //

				while (sBuff[i] != 10 && k < (CE_DESC_S0 - 1))	// unpack description
				  {
					if (sBuff[i] >= ' ')
					  {
						spCE->sDesc[k++]=sBuff[i];
						if ( sBuff[i] < 'A' || sBuff[i] > 'z' ||
							(sBuff[i] > 'Z' && sBuff[i] < 'a')) j++;
					  }
					i++;
				  }

				if ((k-j) < 10)
					spCE->sDesc[0]=0;			// Not enough alphabetic characters to be a description
				else
					spCE->sDesc[k]=0;
			  }
			else
				continue;						// Have a description so ignore commented lines
		  }
	  }

	ut_date_now();								// get current date and time

	CE.bmField=CEF_ID_B0;						// only need to read ID to confirm if this program exists in CE
	CEL.bmField=0;
	if (cef_main(FA_READ+FA_KEY1+FA_STEP, 0) == FA_NODATA_IV0)	// module not found so create a new CE module
	 {
		if (spCE->cLang == 'H')
		 {
			printf("CE: New library module added - %s\n", spCE->sName);
			spCE->iType=CE_LIB_T0;
		 }
		else
		 {
			printf("CE: New program module added - %s\n", spCE->sName);
			spCE->iType=CE_PRG_T0;
		 }

		spCE->iStatus=0;
		spCE->iMDate=spCE->iCDate=gxt_iDate[0];
		spCE->iMTime=spCE->iCTime=gxt_iTime[0];
//		sprintf(spCE->sDesc, "Need to scan source for description");
		spCE->iSize=0;

		CE.bmField=FA_ALL_COLS_B0;				// Going to insert all new fields
		ios=cef_main(FA_WRITE, 0);				// Write a new CE module to db
		ut_check(ios == FA_OK_IV0, "write %d", ios);
	 }
	else										// exits so update last modified date/time
	 {
		spCE->iMDate=gxt_iDate[0];
		spCE->iMTime=gxt_iTime[0];

		CE.bmField=CEF_LAST_MOD_B0+CEF_DESC_B0;	// only need to update last modified date & time
		ios=cef_main(FA_UPDATE+FA_KEY0, 0);		// update CE module in db
		ut_check(ios == FA_OK_IV0, "rewrite %d", ios);
	 }

error:
	if (fp != NULL) fclose(fp);
	cef_main(FA_CLOSE, 0);
	return ios;
  }
