//--------------------------------------------------------------
//
// Scan project config files and update the clice database
//
//	usage:	ce_scan_project
//		To scan the ".project.clice" config file in the current directory i.e. .ce.clice
//		options
//			--version	= version, copyright and licence details
//			--help		= basic help
//
//	GNU GPLv3+ licence	clice - The command-line coding ecosystem by Andrew Bennington 2017 <www.benningtons.net>
//
//--------------------------------------------------------------

#include <ctype.h>		// for tolower
#include <fcntl.h>		// file access functions
#include <string.h>		// for strcopy

#include <fa_def.h>		// file actions
#include <ce_main.h>	// CE database
#include <ut_date.h>	// date and time utilities
#include <ut_error.h>	// for debug and checks

#define	BUFF_S0 200		// Max length of source code lines

int main(int argc, char **argv)
  {

	FILE *fp = NULL;
	char sBuff[BUFF_S0];		// input buffer for init file reads
	int i, j;
	int iSect=0;				// section identifier

	spCE = (struct CE_FIELDS*) &CE;
	spCEL = (struct CEL_FIELDS*) &CEL;

	if (ce_args(argc, argv) < 0) goto error;

	ut_check(cef_main(FA_INIT+FA_OPEN, 0) == 0,	// Initialise libgxtfa and open clice db
			"Failed to open clice db");

	ut_check(getcwd(CE.sDir,
				sizeof(CE.sDir)), "getcwd");	// get current working directory
	for (i=0; i < CE_PROJECT_S0; i++)
		CE.sProject[i]=CE.sDir[strlen(CE.sDir)+i+1-CE_PROJECT_S0];
	CE.sProject[i]='\0';						// extract project code from current directory

	CE.sDesc[0]='\0';							// Mark fields as empty - they may get propulated from the project config file
	CE.sSource[0]='\0';

	sprintf(sBuff, ".xx.clice");				// build config filename to open
	for (i=0; i < CE_PROJECT_S0-1; i++)
		sBuff[i+1]=tolower(CE.sProject[i]);

	ut_date_now();								// get current date and time

	strcpy (CEL.sCode, CE.sProject);
	CEL.iTime=gxt_iTime[0];


	fp = fopen(sBuff, "r");						// Open config file as read-only
	ut_check(fp != NULL, "open config file");	// jumps to error: if not ok

	while (fgets(sBuff, BUFF_S0, fp) != NULL)
	  {
		if (sBuff[0] == '#')	// comment lines start with a hash - so ignore
			continue;
		else if (sBuff[0] == '[')				// start of a new section
		  {
			if (memcmp(&sBuff[1], "Project", 7) == 0)
				iSect=1;
			else if (memcmp(&sBuff[1], "Object Library", 14) == 0)
			  {
				iSect=2;
				CEL.iNtype=CE_OBJT_T0;						// object to object library links
				CEL.iCtype=CE_OLIB_T0;
			  }
			else if (memcmp(&sBuff[1], "Runtime Library", 15) == 0)
			  {
				iSect=3;
				CEL.iNtype=CE_SYSH_T0;						// system headers to run-time library links
				CEL.iCtype=CE_RLIB_T0;
			  }
			else
			  {
				printf("CE: unknown config section ignored: %s\n",
						sBuff);
				iSect=0;
			  }
		  }
		else if (iSect == 1)					// project header section
		  {
			if (memcmp(sBuff,
				"description:", 12) == 0)
			  {
				for (i=12;
						sBuff[i] <= ' '; i++);	// skip white space
				for (j=0; j < CE_DESC_S0 && sBuff[i] != '\n';)
				  {
					if (sBuff[i] != '\'' &&		// guard against accidental SQL injection
						sBuff[i] != '\"' &&
						sBuff[i] != '`')
						CE.sDesc[j++]=sBuff[i];	// extract description
					i++;
				  }
				for (; j < CE_DESC_S0; j++)
					CE.sDesc[j]='\0';			// null terminate
			  }
			else if (sBuff[0] > ' ')
				printf("CE: unknown [Project] setting: %s\n",
						sBuff);
		  }
		else if (iSect == 2 || iSect == 3)		// list of any specified links to libraries
		  {
			CEL.sName[0]=CEL.sCalls[0]='\0';

			for (i=0; i < CE_NAME_S0 && sBuff[i] > ' '; i++)				// extract module name
				CEL.sName[i]=sBuff[i];
			if (i == CE_NAME_S0) ut_error("module name too long");
			CEL.sName[i]='\0';

			while (sBuff[i] != '\n' && sBuff[i] <= ' ') i++;				// skip white space

			for (j=0; j < CE_NAME_S0 && sBuff[i] != '\n'; j++, i++)			// extract library name
				CEL.sCalls[j]=sBuff[i];
			if (j == CE_NAME_S0) ut_error("library name too long");
			CEL.sCalls[j]='\0';

			if (CEL.sName[0] > '\0' && CEL.sCalls[0] > '\0')				// valid entry? #TODO add more validation
			  {
				CE.bmField=0;
				CEL.bmField=CEF_LINK_ID_B0;
				if (cef_main(FA_READ+FA_STEP,
					"cl.name = % AND cl.calls = %") == FA_NODATA_IV0)	// object to library link not found so create a new one
				  {
					CEL.bmField=FA_ALL_COLS_B0;
					i=cef_main(FA_WRITE, 0);				// create new link record in db
				  }
				else										// exists so update last modified time
				  {
					CEL.bmField=CEF_LINK_TIME_B0;
					i=cef_main(FA_UPDATE, "cl.id = %");		// update existing link record in db
				  }
				ut_check(i == FA_OK_IV0, "update %d", i);
				if (iSect == 2) memcpy (CE.sSource, CEL.sCalls, CE_NAME_S0);	// remember project object library for project record
			  }												// #TODO assumes just one object library per project
		  }
	  }

	fclose(fp);									// finished with source code file
	fp=NULL;

	CE.bmField=0;								// Find library links with old timestamps
	CEL.bmField=CEF_LINK_NAME_B0+CEF_LINK_CALLS_B0;
	ut_check(cef_main(FA_READ,
				"cl.code = % AND cl.time <> %") == FA_OK_IV0,
				"purge read");
	i=0;
	while (cef_main(FA_STEP,0) == FA_OK_IV0)
	  {
		printf("clice: no longer linking object %s to library %s\n",
			CEL.sName, CEL.sCalls);
		i=1;
	  }

	if (i == 1)
	  {
		CEL.bmField=FA_ALL_COLS_B0;
		ut_check(cef_main(FA_DELETE,
				"cl.code = % AND cl.time <> %") == FA_OK_IV0,
				"purge");
	  }

	sprintf(CE.sName, "PROJECT");				// setup key for this project
	CE.iType=CE_PROJ_T0;
	CE.cLang=' ';
	CE.cMain=' ';
	CE.cIgnore=' ';
	CE.cLibrary=' ';
	CE.cSpare=' ';
	CE.iSize=0;

	CE.iCDate=CE.iMDate=gxt_iDate[0];
	CE.iCTime=CE.iMTime=gxt_iTime[0];

	CE.bmField=CEF_ID_B0;						// only need to read ID to confirm if this module exists in CE
	CEL.bmField=0;
	if (cef_main(FA_READ+FA_STEP,
		"ce.name = % AND ce.type = % AND ce.project = %") == FA_NODATA_IV0)	// project not found so create a new CE module
	  {
		CE.bmField=FA_ALL_COLS_B0;				// Going to insert all new fields
		i=FA_WRITE;								// Write a new CE module to db
	  }
	else										// exists so update last modified date/time
	  {
		CE.bmField=CEF_LAST_MOD_B0+CEF_DESC_B0+	// update modified date and description (in case changed)
					CEF_SOURCE_B0;				//	also the source field for a PROJECT record that holds any object library name
		i=FA_UPDATE;
	  }

	i=cef_main(i, 0);							// update project record in db
	ut_check(i == FA_OK_IV0, "update %d", i);

error:
	if (fp != NULL) fclose(fp);
	cef_main(FA_CLOSE, 0);
	return 0;
  };
