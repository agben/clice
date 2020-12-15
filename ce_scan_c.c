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
// #TODO - put all user output messages via a function. Use intro title before 1st message and ++, -- on changes. and --quiet option.
//
//	GNU GPLv3+ licence	clice - The command-line coding ecosystem by Andrew Bennington 2017 <www.benningtons.net>
//
//--------------------------------------------------------------

#include <ctype.h>		// for toupper
#include <fcntl.h>		// file access functions
#include <string.h>		// for strcopy

#include <fa_def.h>		// file actions
#include <ce_main.h>	// CE database
#include <ut_date.h>	// date and time utilities
#include <ut_error.h>	// for debug and checks

#define	BUFF_S0 200		// Max length of source code lines
#define	CE_HEADER_M0 40	// Max number of header files in one source file
#define	CE_MODULE_M0 10	// Max number of function/headers written in one source file

int main(int argc, char **argv)
  {
	FILE *fp = 0;
	char sBuff[BUFF_S0];		// input buffer for init file reads
	int i, j, k;
	char *cp;

	int	iHeader = 0;			// number of header files included into source file
	char sHeader[CE_HEADER_M0][CE_NAME_S0];	//List of header files found in source

	int	iType;					// type of source file. i.e. header or program
	int	iModule = 0;			// number of functions/headers written in source file
	char sModule[CE_MODULE_M0][CE_NAME_S0];

	spCE = (struct CE_FIELDS*) &CE;
	spCEL = (struct CEL_FIELDS*) &CEL;



	i=ce_args(argc, argv);
	if (i < 0) goto error;
	ut_check(i == argc-1,					// still an argument remaining? the program to scan
		"Usage = ce_scan_c <source.c>");

	strncpy(	CE.sSource,						// Store source file name
				argv[i],
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

	while (fgets(sBuff, BUFF_S0, fp) != NULL)
	  {
//		ut_debug("in: %s", sBuff);

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
		else if (memcmp(&sBuff[0],
						"#include", 8) == 0)	// found a header file?
		  {
			for (i=8; sBuff[i] != '<' && sBuff[i] != '\n'; i++);
			if (sBuff[i] == '<')				// extract header file name
			  {
				j=0;
				for (i++;	sBuff[i] != '>' &&
							sBuff[i] != '\n' &&
							sBuff[i] != '.'; i++)
					sHeader[iHeader][j++]=sBuff[i];
				sHeader[iHeader][j]='\0';	 	// null terminate string
//				for (; j < CE_NAME_S0; j++)
//					sHeader[iHeader][j]='\0'; 	// null fill remainder of string
				iHeader++;
			  }
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
	CE.cIgnore=' ';
	CE.cLibrary=' ';
	CE.cSpare=' ';
	CE.iCDate=CE.iMDate;
	CE.iCTime=CE.iMTime;
	CE.iSize=0;

	iModule=0;									// count of modules written in this source file
	if (CE.cLang == 'H')						// this is a header file
	  {
		CE.iType=CE_HEAD_T0;
		CE.cMain=' ';
		for (i = 0; i < CE_NAME_S0 &&			// get header name (without the file extension)
						CE.sSource[i] != '\0' &&
						CE.sSource[i] != '.'; i++)
			CE.sName[i]=CE.sSource[i];
		CE.sName[i]='\0';						// null terminate string
//		for (; i < CE_NAME_S0; i++) CE.sName[i]='\0';	// null fill remainder of string

		ut_check(cef_main(FA_ADD, 0) == FA_OK_IV0, "h add CE");

		memcpy(sModule[iModule], CE.sName, CE_NAME_S0);	// retain name for adding/removing links below
		iModule++;
	  }
	else										// a function source file so read through a ctags analysis
	  {
		CE.iType=CE_PROG_T0;

		fp = fopen(sBuff, "r");						// Open ctags file as read-only
		ut_check(fp != NULL, "open ctags file");	// jumps to error: if not ok

		while (fgets(sBuff, BUFF_S0, fp) != NULL)
		  {
			ut_debug("in: %s", sBuff);

			for (i = 0; i < BUFF_S0 && sBuff[i] != ' '; i++);	// skip past item name and
			for (; i < BUFF_S0 && sBuff[i] == ' '; i++);		// skip past following spaces to find item type

			if (memcmp(&sBuff[i], "function", 8) == 0)			// is the item a function definition?
			  {
				if (memcmp(sBuff, "main", 4) == 0)				// replace 'main' modules with the source filename
				  {
					CE.cMain=CE_MAIN_T0;						// Flag as a root program
					for (j = 0; j < CE_NAME_S0 &&
							CE.sSource[j] != '\0' &&
							CE.sSource[j] != '.'; j++)
					  {
						CE.sName[j]=CE.sSource[j];				// #TODO warn and ignore names that are too long
					  }
				  }
				else
				  {
					CE.cMain=' ';
					for (j=0; j < (CE_NAME_S0-1) &&
									sBuff[j] != ' '; j++)		// otherwise copy the item name
					  {
						CE.sName[j]=sBuff[j];
					  }
				  }

				CE.sName[j]='\0';								// null terminate string
//				for (; j < CE_NAME_S0; j++) CE.sName[j]='\0';	// null fill remainder of string

				i+=8;											// skip function
				for (; i < BUFF_S0 && sBuff[i] == ' '; i++);	// skip past following spaces
				for (; i < BUFF_S0 && sBuff[i] != ' '; i++);	// skip past line number
				for (; i < BUFF_S0 && sBuff[i] == ' '; i++);	// skip past following spaces
				for (; i < BUFF_S0 && sBuff[i] != ' '; i++);	// skip past source file name
				for (; i < BUFF_S0 && sBuff[i] == ' '; i++);	// skip past following spaces
				for (j=0; j < (CE_CODE_LINE_S0-1) &&
								sBuff[i] != '\n'; j++)			// Extract function definition
				  {
					if (sBuff[i] == '\'' ||
						sBuff[i] == '\"' ||
						sBuff[i] == '`')						// substitute to prevent an accidental SQL injection!
						CE.sCode[j]='~';
					else
						CE.sCode[j]=sBuff[i];
					i++;
				  }
				CE.sCode[j]='\0';			 					// null terminate string
//				for (; j < CE_CODE_LINE_S0; j++)
//							CE.sCode[j]='\0'; 					// null fill remainder of string

				ut_check(cef_main(FA_ADD, 0) == FA_OK_IV0,
							"f add CE");							// update clice db

				memcpy(sModule[iModule], CE.sName, CE_NAME_S0);	// Keep list of functions found
				iModule++;
			  }
		  }
		if (iModule == 0)
			printf("CE: No modules found in source file?\n");
	  }

	iType=CE.iType;											// beyond here FA_LINK will change CE data so retain source type

	for (i=0; i < iModule; i++)
	  {
		memcpy(CEL.sName, sModule[i], CE_NAME_S0);			// source module names retained in sModule[]
		CEL.iNtype=iType;
		sprintf(CEL.sCode, "not used");
		CEL.iTime=gxt_iTime[0];								// Mark with current time
		for (j=0; j < iHeader; j++)							// Link all source header files to each function
		  {													//	(as we don't know which one each function requires)
			memcpy(CEL.sCalls, sHeader[j], CE_NAME_S0);		// Includes this header file
			CEL.iCtype=CE_HEAD_T0;							// need to keep resetting as FA_LINK will change to system or local header

			ut_check(cef_main(FA_LINK, 0) == FA_OK_IV0, "update CEL");
//#TODO should only display these are new headers for the 1st function - else get a duplicate list for each function displayed
		  }

		CEL.iCtype=CE_HEAD_T0;								//	purge any headers no longer included
		ut_check(cef_main(FA_PURGE,0) == FA_OK_IV0, "purge CEL");
	  }

error:
	if (fp != NULL) fclose(fp);
	cef_main(FA_CLOSE, 0);
	return 0;
  };
