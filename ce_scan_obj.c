//--------------------------------------------------------------
//
// Scan object files for useful info to add to the CE program database
//
//	usage:	ce_scan_obj program.ce
//		where arg1 is the name of the file containing the objdump analysis of the object file
//
//	GNU GPLv3+ licence	CE - Coding Ecosystem by Andrew Bennington 2015 <www.benningtons.net>
//
//--------------------------------------------------------------

#include <fcntl.h>
#include <getopt.h>		// for getopt_long - argument parsing
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fa_def.h>
#include <ce_main.h>
#include <ut_date.h>
#include <ut_error.h>

#define	BUFF_S0 80

int main(int argc, char **argv)
  {
	FILE *fp;
	char sBuff[BUFF_S0];			// input buffer for init file reads
	int i, j;
	int iSection = 0;				// section count through objdump 0=start, 1=Symbols,  2=.text, 3=remainder
//	int iSize = 0;					// size of object file
//	char *cp;

	spCE = (struct CE_FIELDS*) &CE;
	spCEL = (struct CEL_FIELDS*) &CEL;
						// valid arguments: name, has_arg(yes=1, no=0, opt=2), flag, val
	static struct option long_options[] = {
		{"help",	0,	0,		0},	// 0	Keep this order for parsing after getopt_long
		{"version",	0,	0,		0},	// 1
		{NULL,		0,	NULL,	0}
	};

	int option_index = 0;


	while ((i=getopt_long(	argc,					//number of arguments
							argv,					//argument values - an array of pointers to each argument
							"",						//permitted short arguments  i.e. -v (none permitted)
							long_options,			//permitted long arguments   i.e. --version
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
	  }

	ut_check(optind == argc-1,					// still an argument remaining? the object to scan
			"Usage = ce_scan_obj <object.ce>"); // #TODO rather than create a file and pass its name we could read from stdin?

	ut_check(cef_main(FA_INIT+FA_OPEN, 0) == 0,
			"failed to open clice db");			// initialise libgxtfa and open clice db

	ut_date_now();								// get current date & time

	strncpy(spCEL->sName, argv[1], CE_NAME_S0);	// Calling program name
	sprintf(&sBuff[0], "%s.ce", spCEL->sName);	// object filename to be scanned

	fp = fopen(&sBuff[0], "r");					// Open file as read-only
	ut_check(fp != NULL, "open obj");			// jumps to error: if not ok

	while (fgets(sBuff, BUFF_S0, fp) != NULL)
	  {
		ut_debug("in: %s", sBuff);

		if (iSection == 0)
		  {
			if (memcmp(sBuff, "SYMBOL TABLE:", 13) == 0) iSection++;
		  }
		else if (iSection == 1)					// Symbol section
		  {
			if (memcmp(sBuff, "RELOCATION RECORDS FOR [.text]:", 31) == 0)
			  {
				iSection++;
			  }
			else if (sBuff[0] == '0' && sBuff[23] == 'f')	// find source filename
			  {
				j=48;
				for (i=0; i < CE_SOURCE_S0 && sBuff[j] != '\n'; i++)	// sBuff will have a line feed before it terminating null
						spCE->sSource[i]=sBuff[j++];
				for ( ; i < CE_SOURCE_S0; i++) spCE->sSource[i]='\0';	// null fill remainder of string
			  }
			else if (sBuff[0] == '0' && sBuff[23] == 'F')	// find module name(s)
			  {
				j=48;
				if (memcmp(&sBuff[j], "main", 4) == 0)		// replace 'main' modules with the source filename
				  {
					for (i = 0; i < CE_NAME_S0 && spCE->sSource[i] != '\0' && spCE->sSource[i] != '.'; i++)
						spCE->sName[i]=spCE->sSource[i];
				  }
				else
				  {
					for (i=0; i < CE_NAME_S0 && sBuff[j] != '\n'; i++)	// sBuff will have a line feed before it terminating null
						spCE->sName[i]=sBuff[j++];
				  }
				for ( ; i < CE_NAME_S0; i++) spCE->sName[i]='\0';	// null fill remainder of string
				printf("Source=%s  Module=%s\n", spCE->sSource, spCE->sName);

				spCE->iCDate=gxt_iDate[0];						// Update last compiled date and time
				spCE->iCTime=gxt_iTime[0];

				CE.bmField=CEF_ID_B0;							// Select which fields to read
				CEL.bmField=0;
				if (cef_main(FA_READ+FA_KEY1+FA_STEP, 0) == FA_OK_IV0)	// Check if this module exists in clice db?
				  {												// #TODO add warning if module name found in new source file
					i=FA_UPDATE+FA_KEY0;						// Yes so update with current time marker
					CE.bmField=CEF_SOURCE_B0+CEF_LAST_MOD_B0;	// Keep source filename updated
				  }
				else
				  {
					printf("CE: New program module added - %s\n", spCE->sName);
					spCE->iType=CE_PRG_T0;
					spCE->iStatus=0;
					spCE->iMDate=spCE->iCDate;
					spCE->iMTime=spCE->iCTime;
					sprintf(spCE->sDesc, "Need to scan source for description");
					spCE->iSize=0;
					ut_check(getcwd(spCE->sDir,
								sizeof(spCE->sDir)),
								"getcwd");						// get current working directory
					spCE->cLang='C';							// #TODO need to work out the source language from the object dump

					i=FA_WRITE;							// Insert all columns into db
					CE.bmField=FA_ALL_COLS_B0;
				  }
				ut_check(cef_main(i, 0) == FA_OK_IV0, "update CE");

			  }
		  }
		else if (iSection == 2)					//  .text relocation records
		  {
			if (sBuff[0] == 0)
			  {
				break;							// blank line marks the end of the section
			  }
			else
			  {
				continue;
			  }
		  }
	  }


/*
		i=0;
		while (sBuff[i] > ' ')					// unpack referenced module name
		  {
			spCEL->sCalls[i]=sBuff[i];
			i++;
		  }
		spCEL->sCalls[i]=0;
		ut_debug("name=%s", spCEL->sCalls);

		spCEL->cRel=sBuff[++i];					// unpack reference type
		ut_debug("rel=%c", spCEL->cRel);

		if (spCEL->cRel == CEL_REL_UNDAT_V0 ||		// variables and structs - #TODO ignored for now.
			spCEL->cRel == CEL_REL_GDATA_V0 ||
			spCEL->cRel == CEL_REL_LDATA_V0)
			continue;
		else if (spCEL->cRel == CEL_REL_GTEXT_V0)	// symbol for this module (or one of several in this source file)
		  {
			sBuff[i+18]='\0';					// Null terminate for atoi
			cp=&sBuff[i+11];
			iSize+=atoi(cp);					// module size
		  }
		else if (spCEL->cRel == CEL_REL_UNDEF_V0)	// undefined item - likely a link to another clice module or system routine
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
				printf("CE: now using %s\n", spCEL->sCalls);	// No? a new link has been established
				i=FA_WRITE;										// Insert into db
				CEL.bmField=FA_ALL_COLS_B0;
			  }
			spCEL->iTime=gxt_iTime[0];				// Mark with current time
			ut_check(cef_main(i, 0) == FA_OK_IV0, "update CEL");
		  }
		else
			ut_error("type: %c", spCEL->cRel);
	  }


	spCEL->iTime=gxt_iTime[0];				// Check for unused (not time stamped) links
	CE.bmField=0;							// Select which fields to read
	CEL.bmField=CEF_LINK_CALLS_B0;
	ut_check(cef_main(FA_READ, "cl.name = % AND cl.time <> %") == FA_OK_IV0,
			"read CEL");
	i=0;
	while (cef_main(FA_STEP,0) == FA_OK_IV0)
	  {
		printf("CE: no longer using %s\n", spCEL->sCalls);
		i=1;
	  }

	if (i == 1)
	  {
		spCEL->iTime=gxt_iTime[0];			// Purge all remaining unused (not time stamped) links
		CE.bmField=0;						// Select which table to delete from
		CEL.bmField=FA_ALL_COLS_B0;
		ut_check(cef_main(	FA_DELETE,
							"cl.name = % AND cl.time <> %") == FA_OK_IV0,
			"delete CEL");						// jumps to error: if not ok
	  }

	strncpy(spCE->sName, spCEL->sName, CE_NAME_S0);		// Now read main module and update details
	CE.bmField=CEF_ID_B0+CEF_SIZE_B0;			// Select which fields to read
	CEL.bmField=0;
	ut_check(cef_main(FA_READ+FA_STEP,
					"ce.name = %") == FA_OK_IV0,	// read using name as key
		"read CE");									// jumps to error: if not ok

	if (spCE->iSize > 0)
	  {
		i=((iSize-spCE->iSize)*100)/spCE->iSize;		// workout % change in object size
		if (i > 0)
			printf("CE: Ooops grown by %d%%\n", i);
		else if (i < 0)
			printf("CE: Hurrah shrunk by %d%%\n", 0-i);
	  }
	ut_debug("Previous size:%d new size:%d", spCE->iSize, iSize);

	spCE->iSize=iSize;
	spCE->iCDate=gxt_iDate[0];					// Record date & time of last compilation
	spCE->iCTime=gxt_iTime[0];

	CE.bmField=CEF_LAST_COMP_B0+CEF_SIZE_B0;	// Only need to update size & last compiled date & time
	CEL.bmField=0;
	ut_check(cef_main(FA_UPDATE, "ce.id = %") == FA_OK_IV0,		// update using id as key
		"update CE");											// jumps to error: if not ok
*/

error:
	if (fp != NULL) fclose(fp);
	cef_main(FA_CLOSE, 0);

	return 0;
  }
