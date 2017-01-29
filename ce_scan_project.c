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
#include <getopt.h>		// for getopt_long - argument parsing
#include <string.h>		// for strcopy

#include <fa_def.h>		// file actions
#include <ce_main.h>	// CE database
#include <ut_date.h>	// date and time utilities
#include <ut_error.h>	// for debug and checks

#define	BUFF_S0 200		// Max length of source code lines

int main(int argc, char **argv)
  {

	FILE *fp;
	char sBuff[BUFF_S0];		// input buffer for init file reads
	int i, j;
	int iSect=0;				// section identifier

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

	ut_check(cef_main(FA_INIT+FA_OPEN, 0) == 0,	// Initialise libgxtfa and open clice db
			"Failed to open clice db");

	ut_check(getcwd(CE.sDir,
				sizeof(CE.sDir)), "getcwd");	// get current working directory
	for (i=0; i < CE_PROJECT_S0; i++)
		CE.sProject[i]=CE.sDir[strlen(CE.sDir)+i+1-CE_PROJECT_S0];
	CE.sProject[i]='\0';						// extract project code from current directory

	CE.sDesc[0]='\0';							// Mark description field as empty

	sprintf(sBuff, ".xx.clice");				// build config filename to open
	for (i=0; i < CE_PROJECT_S0-1; i++)
		sBuff[i+1]=tolower(CE.sProject[i]);

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
	  }

	fclose(fp);									// finished with source code file
	fp=NULL;

	sprintf(CE.sName, "PROJECT");				// setup key for this project
	CE.iType=CE_PROJ_T0;
	CE.cLang=' ';
	CE.iStatus=0;
	CE.iSize=0;

	ut_date_now();								// get current date and time
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
		CE.bmField=CEF_LAST_MOD_B0+CEF_DESC_B0;	// update modified date and description (in case changed)
		i=FA_UPDATE;
	  }

	i=cef_main(i, 0);							// update project record in db
	ut_check(i == FA_OK_IV0, "update %d", i);

error:
	if (fp != NULL) fclose(fp);
	cef_main(FA_CLOSE, 0);
	return 0;
  };
