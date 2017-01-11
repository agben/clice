//--------------------------------------------------------------
//
// Scan object files for useful info to add to the CE program database
//
//	usage:	objdump -rtl program.o >> program.ce
//			ce_scan_obj program.ce
//		objdump analyses the object file and identifies symbols, global variables, functions, etc... used
//
//	GNU GPLv3+ licence	CE - Coding Ecosystem by Andrew Bennington 2015 <www.benningtons.net>
//
//--------------------------------------------------------------

#include <ctype.h>		// for toupper
#include <fcntl.h>		// file file access functions
#include <getopt.h>		// for getopt_long - argument parsing
#include <stdlib.h>		// for  strtol
#include <string.h>		// for memcmp, strncmp

#include <fa_def.h>		// gxt file access functions
#include <ce_main.h>	// clice global data
#include <ut_date.h>	// gxt date functions
#include <ut_error.h>	// error and debug macros

#define	BUFF_S0 80				// max objdump line width read from file
#define	CE_SCAN_MODULES_M0 50	// max modules one program can link to

int main(int argc, char **argv)
  {
	FILE *fp;					// file pointer
	char sBuff[BUFF_S0];		// input buffer for file reads
	int i, j, k;				// general purpose
	int iSection = 0;			// section count through objdump 0=start, 1=Symbols,  2=.text, 3=remainder

	char sModule[CE_SCAN_MODULES_M0][CE_NAME_S0];	// temp list of module names found in symbols table
	int	iModule = 0;								// Count of how many found

	spCE = (struct CE_FIELDS*) &CE;
	spCEL = (struct CEL_FIELDS*) &CEL;

						// valid arguments: name, has_arg(yes=1, no=0, opt=2), flag, val
	static struct option long_options[] = {
		{"help",	0,	0,		0},	// 0	Keep this order for parsing after getopt_long
		{"version",	0,	0,		0},	// 1
		{NULL,		0,	NULL,	0}};

	int option_index = 0;


	while ((i=getopt_long(	argc,					//number of arguments
							argv,					//argument values - an array of pointers to each argument
							"",						//permitted short arguments  i.e. -v (none permitted)
							long_options,			//permitted long arguments   i.e. --version
							&option_index)) != -1)
	  {
		if (i == '?' || i != 0)				// invalid arg or arg qualifier so abort
			return -1;
		else if (option_index == 0)			// --help
		  {
			ce_help();
			return 0;
		  }
		else if (option_index == 1)			// --version
		  {
			ce_version();
			return 0;
		  }
	  }

	ut_check(optind == argc-1,						// still an argument remaining? the object to scan
		"Usage = ce_scan_obj <object.ce>"); 		// #TODO rather than create a file and pass its name we could read from stdin?

	ut_check(cef_main(FA_INIT+FA_OPEN, 0) == 0,
			"failed to open clice db");				// initialise libgxtfa and open clice db

	ut_date_now();									// get current date & time

	sprintf(&sBuff[0], "%s.ce", argv[1]);			// objdump filename to be scanned

	fp = fopen(&sBuff[0], "r");						// Open file as read-only
	ut_check(fp != NULL, "open obj");				// jumps to error: if not ok

	while (fgets(sBuff, BUFF_S0, fp) != NULL)
	  {
		ut_debug("in: %s", sBuff);

		if (iSection == 0)
		  {
			if (memcmp(sBuff, "SYMBOL TABLE:", 13) == 0) iSection++;
		  }
		else if (iSection == 1)						// Symbol section
		  {
			if (memcmp(sBuff, "RELOCATION RECORDS FOR [.text]:", 31) == 0)
				iSection++;
			else if (sBuff[0] == '0')
			  {
				if (sBuff[23] == 'f')				// find source filename
				  {
					CE.cLang='?';
					j=48;							// start position in objdump output of full source filename
					k=0;							// flag to spot file extension
					for (i=0; i < CE_SOURCE_S0 &&
							sBuff[j] != '\n'; i++)	// sBuff will have a line feed before it terminating null
					  {
						if (k == 0 && sBuff[j] == '.')
							k++;
						else if (k == 1 && sBuff[j] > ' ')
						  {
							CE.cLang=toupper(sBuff[j]);	// use 1st digit of file extension to show language used
							k++;
						  }

						CE.sSource[i]=sBuff[j++];
					  }
					for ( ; i < CE_SOURCE_S0; i++)
						CE.sSource[i]='\0';				// null fill remainder of string

					if (CE.cLang == '?')
						printf("CE: No valid file extension found\n");
				  }
				else if (sBuff[23] == 'F')				// find module name(s)
				  {
					j=48;
					if (memcmp(&sBuff[j], "main", 4) == 0)		// replace 'main' modules with the source filename
						for (i = 0; i < CE_NAME_S0 &&
									CE.sSource[i] != '\0' &&
									CE.sSource[i] != '.'; i++)
							CE.sName[i]=CE.sSource[i];
					else
						for (i=0; i < CE_NAME_S0 && sBuff[j] != '\n'; i++)	// sBuff will have a line feed before a terminating null
							CE.sName[i]=sBuff[j++];
					for ( ; i < CE_NAME_S0; i++) CE.sName[i]='\0';	// null fill remainder of string

					i=strtol(&sBuff[31], NULL, 16);					// Extract module size from hex to dec
					CE.iCDate=gxt_iDate[0];							// Update last compiled date and time
					CE.iCTime=gxt_iTime[0];

					CE.bmField=CEF_ID_B0+CEF_SIZE_B0;				// Select which fields to read
					CEL.bmField=0;
					if (cef_main(FA_READ+FA_KEY1+FA_STEP, 0) == FA_OK_IV0)	// Check if this module exists in clice db?
					  {												// #TODO add warning if module name found in new source file
						if (CE.iSize > 0)
						  {
							j=((i-CE.iSize)*100)/CE.iSize;			// workout % change in object size
							if (j > 0)
								printf("CE: Ooops! %s grown by %d%%\n",
										CE.sName, j);
							else if (j < 0)
								printf("CE: Hurrah! %s shrunk by %d%%\n",
										CE.sName, 0-j);
						  }
						CE.iSize=i;

						i=FA_UPDATE+FA_KEY0;						// Yes so update with current time marker
						CE.bmField=CEF_SOURCE_B0+CEF_LANG_B0+
								CEF_SIZE_B0+CEF_LAST_COMP_B0;	// update the fields we can
					  }
					else
					  {
						printf("CE: New program module added - %s\n", CE.sName);
						CE.iType=CE_PRG_T0;
						CE.iStatus=0;
						CE.iSize=i;
						CE.iMDate=CE.iCDate;
						CE.iMTime=CE.iCTime;
						sprintf(CE.sDesc, "Need to scan source for description");
						ut_check(getcwd(CE.sDir,
								sizeof(CE.sDir)),
								"getcwd");				// get current working directory

						i=FA_WRITE;							// Insert all columns into db
						CE.bmField=FA_ALL_COLS_B0;
					  }
					ut_check(cef_main(i, 0) == FA_OK_IV0, "update CE");

				  }

				if (sBuff[23] == 'F' ||						// find functions declared in this file
					(sBuff[23] == ' ' &&
					memcmp(&sBuff[25], "*UND*", 5) == 0))	// and those declared externally but referenced
				  {
					j=48;									// start position in objdump output of called module name
					for (i=0; i < CE_NAME_S0 &&
							sBuff[j] != '\n'; i++)			// sBuff will have a line feed before it terminating null
						sModule[iModule][i]=sBuff[j++];
					for ( ; i < CE_NAME_S0; i++)
						sModule[iModule][i]='\0';			// null fill remainder of string
					iModule++;
				  }
			  }
		  }
		else if (iSection == 2)							//  .text relocation records
		  {
			if (sBuff[0] == 0)
				break;									// blank line marks the end of the section
			else if (memcmp(sBuff, "OFFSET", 6) == 0)
				continue;								// skip column headers
			else if (sBuff[0] == '0' && sBuff[35] != '.')
			  {
				j=35;									// start position in objdump output of called module name
				for (i=0; i < CE_NAME_S0 &&
							sBuff[j] != '\n' &&
							sBuff[j] != '+' &&
							sBuff[j] != '-'; i++)		// sBuff will have a line feed before it terminating null
					CEL.sCalls[i]=sBuff[j++];
				for ( ; i < CE_NAME_S0; i++)
					CEL.sCalls[i]='\0';					// null fill remainder of string

				for (i=0; i < iModule &&
						strncmp(CEL.sCalls, sModule[i], CE_NAME_S0) != 0;
						i++);
				if (i < iModule)						// matched a function call with one listed in the symbols table
				  {
					CE.bmField=0;						// Select which fields to read
					CEL.bmField=CEF_LINK_ID_B0;			// Just get the ID to confirm if row already exists
					if (cef_main(FA_READ+FA_STEP,
						"cl.name = % AND cl.calls = %") == FA_OK_IV0)	// Link between these modules already exists?
					  {
						i=FA_UPDATE+FA_KEY2;			// Yes so update with current time marker
						CEL.bmField=CEF_LINK_TIME_B0;	// Only need to update the time stamp
					  }
					else
					  {
						printf("CE: %s now using %s\n",
								CEL.sName, CEL.sCalls);	// No? a new link has been established
						sprintf(CEL.sCode, "no code extract");
						CEL.cRel=CEL_REL_UNDEF_V0;
						i=FA_WRITE;						// Insert into db
						CEL.bmField=FA_ALL_COLS_B0;
					  }
					CEL.iTime=gxt_iTime[0];				// Mark with current time
					ut_check(cef_main(i, 0) == FA_OK_IV0, "update CEL");
				  }
			  }
			else if (sBuff[0] > 'A' && sBuff[0] < 'z')	// module name found
			  {
				if (memcmp(sBuff, "main(", 5) == 0)		// replace 'main' modules with the source filename
					for (i = 0; i < CE_NAME_S0 && CE.sSource[i] != '\0' && CE.sSource[i] != '.'; i++)
						CEL.sName[i]=CE.sSource[i];
				else
					for (i=0; i < CE_NAME_S0 &&
								sBuff[i] != '('; i++)	// update current module name
						CEL.sName[i]=sBuff[i];
				for ( ; i < CE_NAME_S0; i++) CEL.sName[i]='\0';		// null fill remainder of string
			  }
		  }
	  }

	CEL.iTime=gxt_iTime[0];					// Check for unused (not time stamped) links
	CE.bmField=0;							// Select which fields to read
	CEL.bmField=CEF_LINK_CALLS_B0;
	ut_check(cef_main(FA_READ, "cl.name = % AND cl.time <> %") == FA_OK_IV0,
			"read CEL");
	i=0;
	while (cef_main(FA_STEP,0) == FA_OK_IV0)
	  {
		printf("CE: no longer using %s\n", CEL.sCalls);
		i=1;
	  }

	if (i == 1)
	  {
		CEL.iTime=gxt_iTime[0];				// Purge all remaining unused (not time stamped) links
		CE.bmField=0;						// Select which table to delete from
		CEL.bmField=FA_ALL_COLS_B0;
		ut_check(cef_main(	FA_DELETE,
							"cl.name = % AND cl.time <> %") == FA_OK_IV0,
			"delete CEL");						// jumps to error: if not ok
	  }

error:
	if (fp != NULL) fclose(fp);
	cef_main(FA_CLOSE, 0);

	return 0;
  }
