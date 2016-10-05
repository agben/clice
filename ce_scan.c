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
//	GNU GPLv3+ licence	clice (CE) - Coding Ecosystem by Andrew Bennington 2015 <www.benningtons.net>
//
//--------------------------------------------------------------

#include <getopt.h>		// for getopt_long - argument parsing
#include <string.h>		// for strcopy

#include <fa_def.h>		// file actions
#include <ce_main.h>	// CE database
#include <ut_date.h>	// date and time utilities
#include <ut_error.h>	// for debug and checks


int main(int argc, char **argv)
  {
	int i;
	int ios=0;

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

	strcpy(spCE->cName, argv[optind]);						// Program name
	ut_check(getcwd(CE.cDir, sizeof(CE.cDir)), "getcwd");	// get current working directory

	ut_debug("Program to scan= %s",spCE->cName);
	ut_debug("Current directory= %s",spCE->cDir);
	ut_debug("language = %c",spCE->cLang);

	ut_date_now();								// get current date and time

	iCEField[CE_MAIN_TABLE_P0]=CEF_ID_B0;		// only need to read ID to confirm if this program exists in CE
	iCEField[CE_LINK_TABLE_P0]=0;
	if (cef_main(FA_READ+FA_KEY1+FA_STEP, 0) == FA_NODATA_IV0)	// module not found so create a new CE module
	 {
		if (spCE->cLang == 'H')
		 {
			printf("CE: New library module added - %s\n", spCE->cName);
			spCE->iType=CE_LIB_T0;
		 }
		else
		 {
			printf("CE: New program module added - %s\n", spCE->cName);
			spCE->iType=CE_PRG_T0;
		 }

		spCE->iStatus=0;
		spCE->iMDate=spCE->iCDate=gxt_iDate[0];
		spCE->iMTime=spCE->iCTime=gxt_iTime[0];
		sprintf(spCE->cDesc, "Need to scan source for description");
		spCE->iSize=0;

		iCEField[CE_MAIN_TABLE_P0]=SQL_ALL_COLS_B0;	// Going to insert all new fields
		ios=cef_main(FA_WRITE, 0);					// Write a new CE module to db
		ut_check(ios == FA_OK_IV0, "write");
	 }
	else											// exits so update last modified date/time
	 {
		spCE->iMDate=gxt_iDate[0];
		spCE->iMTime=gxt_iTime[0];

		iCEField[CE_MAIN_TABLE_P0]=CEF_LAST_MOD_B0;	// only need to update last modified date & time
		iCEField[CE_LINK_TABLE_P0]=0;
		ios=cef_main(FA_UPDATE+FA_KEY0, 0);			// update CE module in db
		ut_check(ios == FA_OK_IV0, "rewrite");
	 }

error:
	cef_main(FA_CLOSE, 0);
	return ios;
  }
