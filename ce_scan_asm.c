//--------------------------------------------------------------
//
// Scan Assemby source files and update the clice database
//
//	usage:	ce_scan_asm <source.asm>
//		where	source.asm	= source filename or library name i.e. library.hsm
//			--version	= version, copyright and licence details
//			--help		= basic help
//
// #TODO - could also be used to check for edit permissions or other warnings?
// #TODO - put all user output messages via a function. Use intro title before 1st message and ++, -- on changes. and --quiet option.
//
//	GNU GPLv3+ licence	clice - The command-line coding ecosystem by Andrew Bennington 2020 <www.benningtons.net>
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
#define	CE_MODULE_M0 50	// Max number of modules included or called in one source file

													// #TODO maybe better to use a pointer to a structure for the variables below
static int iModule = 0;								// Count of how many modules found
static char sModule[CE_MODULE_M0][CE_NAME_S0];		// List of modules included or called found in source
static int iModType[CE_MODULE_M0];					// Flag to show if module type is an included header or called routine


void ce_scan_asm_out(void)
  {
	int i;
	int	iType;					// retained module type
	char sName[CE_NAME_S0];		// retained module name


	if (CE.cLang == 'G')						// #TODO a table to decode file extensions will be needed eventually
		CE.iType=CE_HEAD_T0;					// This is a header file
	else
		CE.iType=CE_PROG_T0;					// else it's code

	ut_check(cef_main(FA_ADD, 0) == FA_OK_IV0, "adding CE");

	memcpy(sName, CE.sName, CE_NAME_S0);			// retain name for adding/removing links below
	iType=CE.iType;									// beyond here FA_LINK will change CE data so retain source type also

	for (i=0; i < iModule; i++)						// Add links to included header files and called routines
	  {
		memcpy(CEL.sName, sName, CE_NAME_S0);		// Keep refreshing these fields as cef_main link and purge could change them
		CEL.iNtype=iType;
		sprintf(CEL.sCode, "not used");
		CEL.iTime=gxt_iTime[0];						// Mark with current time
		memcpy(CEL.sCalls, sModule[i], CE_NAME_S0);
		CEL.iCtype=iModType[i];

		ut_check(cef_main(FA_LINK, 0) == FA_OK_IV0, "update CEL");
	  }

	for (i=CE_PROG_T0; i <=CE_HEAD_T0; i++)			// purge any no longer included functions or headers
	  {
		memcpy(CEL.sName, sName, CE_NAME_S0);		// Keep refreshing these fields as cef_main link and purge could change them
		CEL.iNtype=iType;
		CEL.iCtype=i;
		ut_check(cef_main(FA_PURGE,0) == FA_OK_IV0, "purge CEL");
	  }
error:
	return;
  };



int main(int argc, char **argv)
  {
	FILE *fp = 0;
	char sBuff[BUFF_S0];		// input buffer for init file reads
	int i, j, k;
	char *cp;
	char cLang;					// programming language

	int	iArg;					// Argument counter
	int	init = 1;				// initialise CE record flag

	spCE = (struct CE_FIELDS*) &CE;
	spCEL = (struct CEL_FIELDS*) &CEL;



	iArg=ce_args(argc, argv);					// process any arguments in calling command
	if (iArg < 0) goto error;
	ut_check(iArg == argc-1,					// still an argument remaining? the program to scan
		"Usage = ce_scan_asm <source.asm>");

	ut_check(cef_main(FA_INIT+FA_OPEN, 0) == 0,	// Initialise libgxtfa and open clice db
			"Failed to open clice db");

	CE.sName[0]='\0';
	ut_date_now();								// get current date and time
	strncpy(CE.sSource,							// Store source filename
			argv[iArg],
			CE_SOURCE_S0);
	cp=strrchr(CE.sSource,'.');					// Find last dot in the passed program name before the file extension
	ut_check( cp != NULL,
			"No valid file extension");
	i=cp-CE.sSource;							// size of program name minus file extension
	if (i > CE_NAME_S0) i=CE_NAME_S0;			// #TODO warn about truncated filename
	cLang=toupper(CE.sSource[i+1]);				// initally determine language from file extension A or H
	if (cLang == 'H') cLang='G';				// #TODO a table to decode file extensions will be needed eventually
												// but H is already used for C libraries so code as G instead

	fp = fopen(CE.sSource, "r");				// Open file as read-only
	ut_check(fp != NULL, "open source file");	// jumps to error: if not ok

	while (fgets(sBuff, BUFF_S0, fp) != NULL)
	  {
		if (init == 1)							// Initialise CE fields ready for new function/header
		  {
			strncpy(CE.sSource,					// Store source filename
					argv[iArg],
					CE_SOURCE_S0);
			ut_check(getcwd(CE.sDir,
				sizeof(CE.sDir)), "getcwd");	// get current working directory
			strncpy(CE.sProject,
					&CE.sDir[strlen(CE.sDir)+1-CE_PROJECT_S0],
					CE_PROJECT_S0);				// get project code

			CE.sDesc[0]='\0';
			CE.sCode[0]='\0';
			CE.cMain=' ';
			CE.cLang=cLang;
			CE.cIgnore=' ';
			CE.cLibrary=' ';
			CE.cSpare=' ';
			CE.iCDate=CE.iMDate=gxt_iDate[0];	// Sets last compiled to now to initialise
			CE.iCTime=CE.iMTime=gxt_iTime[0];	//	but these fields are only saved if this is a new module to clice
			CE.iSize=0;
			iModule=0;
			init=0;
		  }

		if (sBuff[0] == ';')					// look for comment lines
		  {
			if (CE.sDesc[0] == '\0')			// Still not found a description
			  {
				j=0;							// check on the number of non-alphabetics used
				k=0;							// output counter
				i=1;							// skip past the comment marker ';'

				int iSpace=0;					// flag to exclude leading or multiple spaces in a description
				while (sBuff[i] != '\n' && k < (CE_DESC_S0 - 1))	// unpack description
				  {
					if (sBuff[i] != '`' &&		// to prevent an accidental SQL injection, ignore these!
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
			else if (CE.sCode[0] == '\0' &&		// Still not found a register summary
					sBuff[1] == 'I' && sBuff[2] =='n')

			  {
				k=0;							// output counter
				i=1;							// skip past the comment marker ';'

				int iSpace=0;					// flag to exclude leading or multiple spaces in a description
				while (sBuff[i] != '\n' &&
					k < (CE_CODE_LINE_S0 - 1))	// unpack register summary
				  {
					if (sBuff[i] != '`' &&		// to prevent an accidental SQL injection, ignore these!
						(sBuff[i] > ' ' ||
						(iSpace == 1 && sBuff[i] == ' ')))
					  {
						CE.sCode[k++]=sBuff[i];
						if (sBuff[i] == ' ')
							iSpace=0;			// had a space so don't allow any more
						else
							iSpace=1;			// had a character so allow a space
					  }
					i++;
				  }
				CE.sCode[k]='\0';
			  }
			else
				continue;						// Already have a description and register summary so ignore commented lines
		  }
		else if (sBuff[0] == '.')				// Skip program label lines
			continue;
		else if (memcmp(&sBuff[0],
						"section", 7) == 0)		// if a section header is found then this must be a 'main' executable program
			CE.cMain=CE_MAIN_T0;
		else if (sBuff[0] >= 'A')				// program or function starting label?
		  {
			for (i = 0; i < CE_NAME_S0 &&		// get name, should have a trailing colon if a program label
					sBuff[i] != '\0' &&
					sBuff[i] != ':'; i++);
			if (sBuff[i] == ':')				// got one
			  {
				if (CE.sName[0] != '\0')		// save details of previous program before starting the next
				  {
					ce_scan_asm_out();
					init=1;						// initialise other CE fields
				  }

				strncpy(CE.sName,
						sBuff,
						i);
//				for (; i < CE_NAME_S0; i++)
//					CE.sName[i]='\0';			// null fill remainder of string
			  }
		  }
		else
		  {
			i=0;										// input parser
			while (sBuff[i] == ' ' ||
				   sBuff[i] == '\t') i++;				// skip any whitespace at the start of lines

			if (memcmp(&sBuff[i],"%include", 8) == 0)	// found a header file?
			  {
				for (i+=8;	i < BUFF_S0 &&
							sBuff[i] != '"' &&
							sBuff[i] != '\n'; i++);
				if (sBuff[i] == '"')					// extract header file name
				  {
					iModType[iModule]=CE_HEAD_T0;
					j=0;
					for (i++;	j < CE_NAME_S0 &&
								sBuff[i] != '"' &&
								sBuff[i] != '.' &&
								sBuff[i] != '\n'; i++)
						sModule[iModule][j++]=sBuff[i];
					sModule[iModule][j]='\0'; 			// null terminate string
//					for (; j < CE_NAME_S0; j++)
//						sModule[iModule][j]='\0'; 		// null fill remainder of string
					if (sBuff[i] == '.' &&
						sBuff[i+1] == 'h') iModule++;	// ignore all but .hsm header files
				  }
			  }
			else if (memcmp(&sBuff[i],"call", 4) == 0)	// found a call to a routine?
			  {
				i+=4;
				while (sBuff[i] == ' ' ||
					   sBuff[i] == '\t') i++;			// skip any whitespace
				if (sBuff[i] > ' ')						// extract routine name
				  {
					iModType[iModule]=CE_PROG_T0;
					j=0;
					while (j < CE_NAME_S0 && sBuff[i] > ' ')
						sModule[iModule][j++]=sBuff[i++];
					sModule[iModule][j]='\0'; 			// null terminate string
//					for (; j < CE_NAME_S0; j++)
//						sModule[iModule][j]='\0'; 		// null fill remainder of string
					iModule++;
				  }
			  }
		  }
	  }

	fclose(fp);									// finished with source code file
	fp=NULL;

	if (CE.sName[0] == '\0')		// No program label found so use the name of the source file
	  {
	  for (i=j=0; i < CE_NAME_S0; i++)
			if (CE.sSource[j] != '.')
				CE.sName[i]=CE.sSource[j++];
			else
				CE.sName[i]='\0';
	  }
	ce_scan_asm_out();

error:
	if (fp != NULL) fclose(fp);
	cef_main(FA_CLOSE, 0);
	return 0;
  };
