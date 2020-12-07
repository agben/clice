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
#include <stdlib.h>		// for  strtol
#include <string.h>		// for memcmp, strncmp

#include <fa_def.h>		// gxt file access functions
#include <ce_main.h>	// clice global data
#include <ut_date.h>	// gxt date functions
#include <ut_error.h>	// error and debug macros

#define	BUFF_S0 80					// max objdump line width read from file
#define	CE_SCAN_MODULE_M0 50		// max modules one program can link to
#define	CE_MODULE_M0 10				// max number of functions written within one source file

int main(int argc, char **argv)
  {
	FILE *fp = 0;									// file pointer
	char sBuff[BUFF_S0];							// input buffer for file reads
	int i, j, k;									// general purpose
	int iSection = 0;								// section count through objdump 0=start, 1=Symbols,  2=.text, 3=remainder

	char sModule[CE_SCAN_MODULE_M0][CE_NAME_S0];	// temp list of module names found in symbols table
	int	iModule = 0;								// Count of how many found
	char sSource[CE_NAME_S0];						// name of source file (without path or extension)
	int	iFunc = 0;									// Count of how many functions written in this source file
	char sFunc[CE_MODULE_M0][CE_NAME_S0];			// and a list of their function names (with main() converted to source name)


	spCE = (struct CE_FIELDS*) &CE;
	spCEL = (struct CEL_FIELDS*) &CEL;


	i=ce_args(argc, argv);											// parse command arguments i.e. ce_scan_obj --version
	if (i < 0) goto error;
	ut_check(i == argc-1,											// still an argument remaining? the object to scan
		"Usage = ce_scan_obj <object.ce>");

	ut_check(cef_main(FA_INIT+FA_OPEN, 0) == 0,
			"failed to open clice db");								// initialise libgxtfa and open clice db

	ut_date_now();													// get current date & time

	sprintf(&sBuff[0], "%s.ce", argv[1]);							// objdump filename to be scanned

	fp = fopen(&sBuff[0], "r");										// Open file as read-only
	ut_check(fp != NULL, "open obj");								// jumps to error: if not ok

	while (fgets(sBuff, BUFF_S0, fp) != NULL)
	  {
		ut_debug("in: %s", sBuff);

		if (iSection == 0)											// Section 0 - at the start and looking for the symbol table
		  {
			if (memcmp(sBuff, "SYMBOL TABLE:", 13) == 0) iSection++;
		  }
		else if (iSection == 1)										// Section 1 - Symbol section - get symbol list and source module names
		  {
			if (memcmp(sBuff, "RELOCATION RECORDS FOR [.text]:", 31) == 0)
			  {
				iSection++;											// end of section 1 so preserve CE details that are needed later
				for (i=0; i < CE_NAME_S0 &&
							CE.sSource[i] != '\0' &&
							CE.sSource[i] != '.'; i++)
					sSource[i]=CE.sSource[i];						// keep source name for use in next section
				sSource[i]='\0';									// null terminate string
			  }
			else if (sBuff[0] == '0')								// a symbol definition?
			  {
				if (sBuff[23] == 'f')								// find source filename
				  {
					CE.cLang='?';
					j=48;											// start position in objdump output of full source filename
					k=0;											// flag to spot file extension
					for (i=0; i < CE_SOURCE_S0 &&
							sBuff[j] != '\n'; i++)					// sBuff will have a line feed before it terminating null
					  {
						if (k == 0 && sBuff[j] == '.')
							k++;
						else if (k == 1 && sBuff[j] > ' ')
						  {
							CE.cLang=toupper(sBuff[j]);				// use 1st digit of file extension to show language used
							k++;
						  }

						CE.sSource[i]=sBuff[j++];
					  }
					CE.sSource[i]='\0';								// null terminate string

					if (CE.cLang == '?')
						printf("CE: No valid file extension found\n");
				  }
				else if (sBuff[23] == 'F')							// find module name(s)
				  {
					j=48;
					if (memcmp(&sBuff[j], "main", 4) == 0)			// replace 'main' modules with the source filename
						for (i = 0; i < CE_NAME_S0 &&
									CE.sSource[i] != '\0' &&
									CE.sSource[i] != '.'; i++)
							CE.sName[i]=CE.sSource[i];
					else
						for (i=0; i < CE_NAME_S0 && sBuff[j] != '\n'; i++)	// sBuff will have a line feed before a terminating null
							CE.sName[i]=sBuff[j++];
					CE.sName[i]='\0';								// null terminate string

					if (iFunc < CE_MODULE_M0-1)
						memcpy(sFunc[iFunc++], CE.sName, CE_NAME_S0);	// keep a list of written function names
					else
						printf("CE: Too many functions in one source file! Ignoring: %s\n",
								CE.sName);

					i=strtol(&sBuff[31], NULL, 16);					// Extract module size from hex to dec
					CE.iCDate=gxt_iDate[0];							// Update last compiled date and time
					CE.iCTime=gxt_iTime[0];

					CE.bmField=CEF_ID_B0+CEF_SIZE_B0;				// Select which fields to read
					CEL.bmField=0;
					if (cef_main(FA_READ+FA_KEY1+FA_STEP, 0) == FA_OK_IV0)	// Check if this module exists in clice db?
					  {
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
								CEF_SIZE_B0+CEF_LAST_COMP_B0;		// update the fields we can
					  }
					else
					  {
						printf("CE: New program module added - %s\n", CE.sName);
						CE.iType=CE_PROG_T0;
						CE.cMain=' ';
						CE.cIgnore=' ';
						CE.cLibrary=' ';
						CE.cSpare=' ';
						CE.iSize=i;
						CE.iMDate=CE.iCDate;
						CE.iMTime=CE.iCTime;
						sprintf(CE.sDesc, "Need to scan source for description");
						ut_check(getcwd(CE.sDir,
								sizeof(CE.sDir)),
								"getcwd");							// get current working directory

						i=FA_WRITE;									// Insert all columns into db
						CE.bmField=FA_ALL_COLS_B0;
					  }
					ut_check(cef_main(i, 0) == FA_OK_IV0, "update CE");

				  }

				if (sBuff[23] == 'F' ||								// find functions declared in this file
					(sBuff[23] == ' ' &&
					memcmp(&sBuff[25], "*UND*", 5) == 0))			// and those declared externally but referenced
				  {
					j=48;											// start position in objdump output of called module name
					for (i=0; i < CE_NAME_S0 &&
							sBuff[j] != '\n'; i++)					// sBuff will have a line feed before it terminating null
						sModule[iModule][i]=sBuff[j++];
					sModule[iModule][i]='\0';						// null terminate string
					iModule++;
				  }
			  }
		  }
		else if (iSection == 2)										// Section 2 - .text relocation records - identify all function calls
		  {															// CE data changed from here by FA_LINK calls
			if (sBuff[0] == 0)
				break;												// blank line marks the end of the section
			else if (memcmp(sBuff, "OFFSET", 6) == 0)
				continue;											// skip column headers
			else if (sBuff[0] == '0' && sBuff[35] != '.')
			  {
				j=35;												// start position in objdump output of called module name
				for (i=0; i < CE_NAME_S0 &&
							sBuff[j] != '\n' &&
							sBuff[j] != '+' &&
							sBuff[j] != '-'; i++)					// sBuff will have a line feed before it terminating null
					CEL.sCalls[i]=sBuff[j++];
				CEL.sCalls[i]='\0';									// null terminate string

				for (i=0; i < iModule &&
						strncmp(CEL.sCalls, sModule[i], CE_NAME_S0) != 0;
						i++);
				if (i < iModule)									// matched a function call with one listed in the symbols table
				  {
					sprintf(CEL.sCode, "no code extract");
					CEL.iNtype=CE_PROG_T0;							// program to program link
					CEL.iCtype=CE_PROG_T0;
					CEL.iTime=gxt_iTime[0];							// Mark with current time

					ut_check(cef_main(FA_LINK, 0) == FA_OK_IV0, "update CEL");
				  }
			  }
			else if (sBuff[0] > 'A' && sBuff[0] < 'z')				// module name found
			  {
				if (memcmp(sBuff, "main(", 5) == 0)					// replace 'main' modules with the source filename
					memcpy(CEL.sName, sSource, CE_NAME_S0);
				else
				  {
					for (i=0; i < CE_NAME_S0 &&
								sBuff[i] != '('; i++)				// update current module name
						CEL.sName[i]=sBuff[i];
					CEL.sName[i]='\0';								// null terminate string
				  }
			  }
		  }
	  }

	for (i=0; i < iFunc; i++)
	  {
		strncpy(CEL.sName, sFunc[i], CE_NAME_S0);
		CEL.iTime=gxt_iTime[0];										// Check for unused (not time stamped) links
		CEL.iNtype=CE_PROG_T0;
		CEL.iCtype=CE_PROG_T0;
		ut_check(cef_main(FA_PURGE, 0) == FA_OK_IV0,
			"purge CEL");
	  }

error:
	if (fp != NULL) fclose(fp);
	cef_main(FA_CLOSE, 0);

	return 0;
  }
