//--------------------------------------------------------------
//
//	clice - the command line coding ecosystem
//
//	usage:	./ce_clice	or define as the command 'clice'
//
//	GNU GPLv3+ licence	clice by Andrew Bennington Jan-2016 <www.benningtons.net>
//
//--------------------------------------------------------------

#include <ctype.h>		// common I/O functions like toupper()
#include <stdlib.h>		// memory management
#include <string.h>		// for memcpy

#include <fa_def.h>		// filehandler actions
#include <nc_main.h>	// NC ncurses utility definitions
#include <ce_main.h>	// CE database definitions
#include <ut_error.h>	// error checking and debug macros


#define CE_DISP_M0	3				// Lines in an item display
#define CE_LIST_M0	50				// Max search results to display

#define CE_PROJECTS_MENU_P0	6		// offsets for each list of menu options
#define CE_ACTIONS_MENU_P0	11
char *cpMenu[] =	{	"1) My Projects",
						"2) My Programs",
						"3) My Header files",
						"4) System Functions",
						"5) System Headers",
			(char *) NULL,
						"1) Next",
						"2) Previous",
						"!3) Make all",
						"!4) Make install",
			(char *) NULL,
						"1) Next",
						"2) Previous",
						"3) Links to",
						"4) Links from",
						"!5) Edit",
						"6) Remove",
						"!7) Make",
			(char *) NULL};

//--------------------------------------------------------------
//
//	Manage projects with clice
//
//--------------------------------------------------------------

void ce_clice_project(void)
  {
	int	i;
	int	iOpt;					// selected menu option
	int iPos;					// position within a search list
	int iHits;					// number of hits found by each search
	char sBuff[CE_DESC_S0+10];	// string buffer

    char sList[CE_LIST_M0][CE_PROJECT_S0+3+CE_DESC_S0];	// list of their names
    char *cpList[CE_LIST_M0];



	CE.bmField=CEF_PROJECT_B0;					// Get a list of projects from the clice db
	CEL.bmField=0;
	CE.sProject[0]='\0';						// #TODO have to provide a key for a mandatory WHERE ... , but why?
	ut_check(cef_main(FA_READ+FA_DISTINCT,
				"ce.project > %") == FA_OK_IV0,
					"Read projects");			// jump to error: if SQL prepare fails.
	iHits=0;
	while (cef_main(FA_STEP, 0) == FA_OK_IV0)
	  {
		cpList[iHits]=sList[iHits];				// establish an array of pointers for nc_menu
		snprintf(	sList[iHits],				//	and a list of known projects with any descriptions
					CE_PROJECT_S0,
					"%s",CE.sProject);
		iHits++;
		if (iHits == CE_LIST_M0)
		  {
			iHits--;
			nc_message("Too many projects - showing first page only");
			sleep(2);
			break;
		  }
	  }

	if (iHits == 0)								// no projects found in clice db
	  {
		nc_message("No projects found");
		sleep(2);
	  }
	else
	  {
		cpList[iHits]=(char *) NULL;			// mark end of projects list
		for (i=0; i < iHits; i++)				// check for any project descriptions to add to menu
		  {
			CE.bmField=CEF_DESC_B0;
			CEL.bmField=0;
			CE.iType=CE_PROJ_T0;
			snprintf(	CE.sProject,
						CE_PROJECT_S0,
						"%s", sList[i]);
			if (cef_main(FA_READ+FA_STEP,
					"ce.type = % AND ce.project = %") == FA_OK_IV0)
				snprintf(	sList[i],
							CE_PROJECT_S0+3+CE_DESC_S0,
							"%s   %s",
							CE.sProject, CE.sDesc);
		  }

		iOpt=iPos=(iHits == 1) ? 1 :
			nc_menu("Select required entry",
					cpList);					// display project list until selection or quit requested
		while (iOpt != NC_QUIT)
		  {
			CE.iType=CE_PROJ_T0;				// setup to read the project's details
			for (i=0; i < CE_PROJECT_S0 &&
					sList[iPos-1][i] > ' '; i++)
					CE.sProject[i]=sList[iPos-1][i];
			CE.sProject[i]='\0';

			snprintf(sBuff, sizeof(sBuff), "Project:%s", cpList[iPos-1]);
			iOpt=nc_menu(sBuff, cpMenu+CE_PROJECTS_MENU_P0);
			switch (iOpt)
			  {
				case 1:							// Next item
					if (iPos < iHits) iPos++;
					break;
				case 2:							// Previous item
					if (iPos-1 > 0) iPos--;
					break;
				case 3:
//					ce_make_all();
					break;
				case 4:
//					ce_make_install();
					break;
			  }
		  }
	  }
error:;
  };


//--------------------------------------------------------------
//
//	Manage modules (programs & headers) with clice
//
//--------------------------------------------------------------

void ce_clice_module(void)
  {
	int	i, j;
	char *cp;
	int	ios;					// io status
	int	iOpt = -1;				// selected menu option
	int	iPick;					// selected list option
	int iPos;					// position within a search list

								// The following variables hold a master list [0] and a proposed master list [1]
								// i.e. We have an established list of modules but may decide to switch that for the proposed list
	int iHits[2];				// number of hits found by each search

//#TODO	should use malloc for these?
    int iList[CE_LIST_M0][2];					// list of selected clice db items
    char sList[CE_LIST_M0][2][CE_NAME_S0+20];	// list of their names
    char *cpList[CE_LIST_M0];
	char sBuff[CE_DISP_M0*CE_DESC_S0];			// string buffer
	char sType[20];								// type of module



	nc_message("Use % for wildcard");

	i=nc_input(	"Select required module",
				CE.sName,
				CE_NAME_S0-1);					// set name to look for in clice db (-1 to ensure room for trailing null)
	i=strnlen(CE.sName, CE_NAME_S0-1);
	if (i < CE_NAME_S0-1) CE.sName[i++]='%';	// if room add a default wildcard
	CE.sName[i]='\0';

	CE.bmField=CEF_ID_B0+CEF_NAME_B0;			// What to read from the ce database
	CEL.bmField=0;
	ut_check(cef_main(FA_READ+FA_KEY5,
						0) == FA_OK_IV0,		// prepare a select of all matching entries
					"Read key5");				// jump to error: if SQL prepare fails.
	iHits[0]=0;
	while (cef_main(FA_STEP, 0) == FA_OK_IV0)
	  {
		cpList[iHits[0]]=sList[iHits[0]][0];	// establish an array of pointers for nc_menu
		iList[iHits[0]][0]=CE.iNo;
		sprintf(sList[iHits[0]++][0],"%s",CE.sName);	// pointers to the name of each matching record

		if (iHits[0] == CE_LIST_M0)
		  {
			iHits[0]--;
			nc_message("Too many matches - showing first page only");
			sleep(2);
			break;
		  }
	  }

	if (iHits[0] == 0)							// no matches in clice database
	  {
		nc_message("No matches found");
		sleep(2);
	  }
	else
	  {
		cpList[iHits[0]]=(char *) NULL;			// mark end of search results list

		iOpt=iPos=(iHits[0] == 1) ? 1 :
			nc_menu("Select required entry",
					cpList);					// display search results until selection or quit requested
		while (iOpt != NC_QUIT)
		  {
			CE.iNo=iList[iPos-1][0];			// Read more about the selected item
			CE.bmField=CEF_DESC_B0+CEF_LANG_B0+
					CEF_CODE_B0+CEF_TYPE_B0;	// What to read from the ce database
			CEL.bmField=0;
			ut_check(cef_main(FA_READ+FA_STEP,
							0) == FA_OK_IV0,	// prepare a select for selected item
						"Read key0");			// jump to error: if SQL prepare fails.

			switch (CE.iType)					// display item type
			  {
				case CE_PROG_T0:
				case CE_HEAD_T0:
					if (CE.cLang == 'S')
					  {
						sprintf(sType,"Shell script");
						break;
					  }
					if (CE.cLang == 'C' || CE.cLang == 'H')
						sType[0] = 'C';
					if (CE.iType == CE_PROG_T0)
						sprintf(&sType[1]," program");
					else if (CE.iType == CE_HEAD_T0)
						sprintf(&sType[1]," header");
					else
						sprintf(&sType[1]," unknown");
					break;
				case CE_SYSF_T0:
					sprintf(&sType[0],"system function");
					break;
				case CE_SYSH_T0:
					sprintf(&sType[0],"system header");
					break;
				default:
					sprintf(sType,"?? %c",CE.cLang);
			  }

			for (i=0; sList[iPos-1][0][i] > ' ' && i < CE_NAME_S0; i++);	//Find length of item name
			j=snprintf(sBuff, sizeof(sBuff), "%-*.*s    (%s)\n",
					i, i,
					sList[iPos-1][0],
					sType);

			if (CE.iType < CE_SYSF_T0)			// local function/header?
				j+=snprintf(&sBuff[j], sizeof(sBuff)-j, "%s", CE.sDesc);
			sBuff[j++]='\n';

			if (CE.iType > CE_HEAD_T0 || CE.cLang == 'H' || CE.cLang == 'S')
				sBuff[j++]='\0';				// no example code to display for header or script files
			else
				j+=snprintf(&sBuff[j], sizeof(sBuff)-j, "%s", CE.sCode);
			iOpt=nc_menu(sBuff, cpMenu+CE_ACTIONS_MENU_P0);
			switch (iOpt)						// then check for menu selection actions
			  {
				case 1:							// Next item
					if (iPos < iHits[0]) iPos++;
					break;

				case 2:							// Previous item
					if (iPos-1 > 0) iPos--;
					break;

				case 3:							// Calls to
				case 4:							// Called from
					CE.bmField=0;				// need link details not module details
					if (iOpt == 3)
					  {
						CEL.iNtype=CE.iType;	// type of current item
						for (i=0; sList[iPos-1][0][i] > ' '; i++)
							CEL.sName[i]=sList[iPos-1][0][i];
						CEL.sName[i]='\0';
						CEL.bmField=CEF_LINK_CALLS_B0+CEF_LINK_CTYPE_B0;	// read list of items linked from this one
						ut_check(cef_main(FA_READ,
							"cl.name = % AND cl.ntype = % ORDER BY cl.calls ASC") == FA_OK_IV0,	// prepare a select of all modules linked
							"Read links to");			// to or jump to error: if fails.
					  }
					else
					  {
						CEL.iCtype=CE.iType;			// type of current item
						for (i=0; sList[iPos-1][0][i] > ' '; i++)
							CEL.sCalls[i]=sList[iPos-1][0][i];
						CEL.sCalls[i]='\0';
						CEL.bmField=CEF_LINK_NAME_B0+CEF_LINK_NTYPE_B0;		// read list of items that link to this one
						ut_check(cef_main(FA_READ,
							"cl.calls = % AND cl.ctype = % ORDER BY cl.name ASC") == FA_OK_IV0,	// prepare a select of all modules linked
							"Read links from");			// from or jump to error: if fails.
					  }

					int iLen=0;							// Find longest name in list
					iHits[1]=0;
					while (cef_main(FA_STEP, 0) == FA_OK_IV0)
					  {
						cpList[iHits[1]]=sList[iHits[1]][1];	// establish an array of pointers for nc_menu - #TODO must be a better way?
						if (iOpt == 3)							// called modules
						  {
							cp=CEL.sCalls;
							i=CEL.iCtype;
						  }
						else									// calling modules
						  {
							cp=CEL.sName;
							i=CEL.iNtype;
						  }
							sprintf(sList[iHits[1]][1], "%-*s   %s",
								CE_NAME_S0,
								cp,
								(i == CE_PROG_T0) ?	"function":
								(i == CE_HEAD_T0) ?	"header":
								(i == CE_SYSF_T0) ?	"system function":
								(i == CE_SYSH_T0) ?	"system Header":
													"unknown");
						j=strnlen(cp, CE_NAME_S0);
						if (j > iLen) iLen=j;			// check if this is the longest name?

						iList[iHits[1]++][1]=0;			// Will determine if each called item exists in clice later
						if (iHits[1] == CE_LIST_M0)		//#TODO need to handle large lists better - this will keep pausing
						  {
							iHits[1]--;
							nc_message("Too many matches - showing first page only");
							sleep(2);
							break;
						  }
					  }

					if (iHits[1] == 0)					// no matches in clice database
					  {
						if (iOpt == 3)
							nc_message("No linked items found in clice");
						else
							nc_message("Not linked from anything in clice");
						sleep(2);			// iList[][0], sList[][0], iPos and iHits[0] are preserved so just continue.
					  }
					else
					  {
						cpList[iHits[1]]=(char *) NULL;	// mark end of search results list
						iLen+=3;						// allow this many spaces between name and type in the list

						for (i=0; i < iHits[1]; i++)
						  {
							for(j=0; sList[i][1][j] > ' '; j++)
								CE.sName[j]=sList[i][1][j];
							CE.sName[j]='\0';			// extract item name and null terminate

							for (j=iLen; sList[i][1][j] != '\0'; j++)
								sList[i][1][j]=sList[i][1][j+(CE_NAME_S0-iLen)];	// reduce blank space in output list

							CE.bmField=CEF_ID_B0;		// See which called modules exist in clice
							CEL.bmField=0;
							ut_check(cef_main(FA_READ+FA_KEY1,		// #TODO should read by name + type
												0) == FA_OK_IV0,	// prepare a select for selected item
									"Read key1");		// jump to error: if SQL prepare fails.

							if (cef_main(FA_STEP,0) == FA_OK_IV0)
								iList[i][1]=CE.iNo;
							else						// Mark module as unknown in clice by inserting a ! marker
							  {
								sList[i][1][sizeof(sList[0][0])-1]='\0';	// ensure null termination of string
								for(j=sizeof(sList[0][0])-2; j > 0; j--)
									sList[i][1][j]=sList[i][1][j-1];
								sList[i][1][0]='!';
							  }
						  }

						iPick=nc_menu("Select required entry",
										cpList);					// display search results until selection or quit requested
						if (iPick != NC_QUIT)						// Switch master search list with our proposed list
						  {
							iHits[0]=0;
							for (i=0; i < iHits[1]; i++)
							  {
								if (sList[i][1][0] != '!')		// pick only the entries that are recorded in the clice db
								  {
									iList[iHits[0]][0]=iList[i][1];
									memcpy(	sList[iHits[0]][0],
											sList[i][1],
											sizeof(sList[0][0]));
									iHits[0]++;
									if (i == iPick-1) iPos=iHits[0];	// position of selected item within the new (clice db entries only) list
								  }
							  }
//										iPos=i;
//										iHits[0]=iHits[1];
//										for (i=0; i<iHits[0]; i++)
//										  {
//											iList[i][0]=iList[i][1];
//											memcpy(	sList[i][0],
//													sList[i][1],
//													CE_NAME_S0);
//										  }
						  }
					  }
					break;

				case 6:					// Remove an item and all it's links from the clice db

					CE.bmField=CEF_ID_B0;
					CEL.bmField=0;
					ios=cef_main(FA_DELETE, 0);					// 1st delete main clice db row for this id
					ut_check(ios == FA_OK_IV0, "Delete main %d", ios);

					CE.bmField=0;
					CEL.bmField=CEF_LINK_CALLS_B0;
					memcpy(CEL.sName,sList[iPos-1][0],CE_NAME_S0);
					memcpy(CEL.sCalls, CEL.sName, CE_NAME_S0);
					ios=cef_main(FA_DELETE, "cl.name = % OR cl.calls = %");		// 2nd delete all links to+from this item
					ut_check(ios == FA_OK_IV0, "Delete links %d", ios);

					nc_message("Item has been removed from the clice db");
					sleep(2);

					iOpt=NC_QUIT;				// change action to ensure menus behave

					break;

				case NC_QUIT:					// Quit item display
					if (iHits[0] > 1)			// if only 1 hit then quit to main menu else
					  {
						iPos=nc_menu("Select required entry",
										cpList);		// display search results until selection or quit request
						if (iPos != NC_QUIT) iOpt=1;	// Pass on a quit request from the search results list
//#TODO if returning to the search results list then it would be nice to retain our current position rather than return to the top
					  }
					break;
			  }
		  }
	  }

error:;
  };


//--------------------------------------------------------------
//
//	clice - command line coding ecosystem management menu
//
//--------------------------------------------------------------

int main(int argc, char **argv)
  {
	int	i;
	int	iOpt = -1;				// selected menu option

	char cMenu[CE_SYSH_T0+1][30];	// currently one menu item per module type with system headers the last (SYSH)
    char *cpList[CE_SYSH_T0+2];		// +2 to allow for a null pointer to terminate the menu list


	if (ce_args(argc, argv) < 0) goto error;	// process any command arguments i.e. clice --version

	ut_check(cef_main(FA_INIT+FA_OPEN, 0) == 0,
			"Open ce_main.db");					// initialise libgxtfa and open clice database

	nc_start();									// initialise libgxtnc and startup ncurses screen display

	while (iOpt != NC_QUIT)						// Display and manage menu until requested to quit
	  {
		for (CE.iType=CE_PROJ_T0; CE.iType <= CE_SYSH_T0; CE.iType++)
		  {
			cef_main(FA_COUNT, "ce.type = %");	// Create menu with up to date counts of module types
			i=0;
			if (CE.iCount <= 0) cMenu[CE.iType][i++]='!';	// No items so disable this option
			i+=sprintf (&cMenu[CE.iType][i], "%s", *(cpMenu+CE.iType));
			if (CE.iCount > 0) sprintf (&cMenu[CE.iType][i], " [%d]", CE.iCount);
			cpList[CE.iType]=cMenu[CE.iType];
		  }
		cpList[CE.iType]=NULL;

		switch ((iOpt=nc_menu("clice Menu",cpList)))
		  {
			case 1:
				ce_clice_project();
				break;
			case 2:
			case 3:
			case 4:
			case 5:
				CE.iType=iOpt-1;			// set type of item required (program, header, ...)
				ce_clice_module();
				break;
		  }
	  }

error:							// tidily stop
	nc_stop();					// end ncurses
	cef_main(FA_CLOSE, 0);

	return 0;
  }
