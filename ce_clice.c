//--------------------------------------------------------------
//
//	clice - command line coding environment (CE) management menu
//
//	usage:	./ce_clice	or define as the command 'clice'
//
//	GNU GPLv3+ licence	clice by Andrew Bennington Jan-2016 <www.benningtons.net>
//
//--------------------------------------------------------------

#include <ctype.h>		// common I/O functions like toupper()
#include <getopt.h>		// for getopt_long - argument parsing
#include <stdlib.h>		// memory management

#include <fa_def.h>		// filehandler actions
#include <nc_main.h>	// NC ncurses utility definitions
#include <ce_main.h>	// CE database definitions
#include <ut_error.h>	// error checking and debug macros


#define CE_DISP_M0	5				// Lines in an item display
#define CE_LIST_M0	50				// Max search results to display

#define CE_PROJECTS_TITLE_P0	2	// offsets for each menu title
#define CE_PROG_TITLE_P0	4
#define CE_SELECT_TITLE_P0	10

char *cpTitle[] =	{	"clice Menu",
			(char *) NULL,
						"Enter project name",
			(char *) NULL,
						"Enter program name",
			(char *) NULL,
						"Enter system routine name",
			(char *) NULL,
						"Enter library name",
			(char *) NULL,
						"Select required entry",
			(char *) NULL};

#define CE_PROJECTS_MENU_P0	5		// offsets for each list of menu options
#define CE_ACTIONS_MENU_P0	8
char *cpMenu[] =	{	"1) Projects",
						"2) Programs",
						"3) System Routines",
						"4) Libraries",
			(char *) NULL,
						"1) Make all",
						"2) Make install",
			(char *) NULL,
						"1) Next",
						"2) Previous",
						"3) Edit",
						"4) Calls to",
						"5) Calls from",
						"6) Delete",
						"7) Make",
			(char *) NULL};


int main(int argc, char **argv)
  {
	static struct option long_options[] = { 		// valid arguments: name, has_arg(yes=1, no=0, opt=2), flag, val
					{"help",	0,	0,	0},			// 0	Keep this order for parsing after getopt_long
					{"version",	0,	0,	0},			// 1
					{NULL,		0,	NULL,	0}
	};

	int option_index = 0;
	int	i;
	int	iOpt;								// selected menu option
	int iPos;								// position within a search list

//#TODO	should use malloc for these?
    char iList[CE_LIST_M0];					// list of selected clice db items
    char sList[CE_LIST_M0][CE_NAME_S0+10];	// list of their names
    char *cpList[CE_LIST_M0];

    char sDisp[CE_DISP_M0][CE_DESC_S0+10];	// bespoke menu/list for display
    char *cpDisp[CE_DISP_M0];



    while ((i=getopt_long(	argc,					//number of arguments
							argv,					//argument values - an array of pointers to each argument
							"",						//permitted short arguments  i.e. -v (none permitted)
							long_options,			//permitted long arguments   i.e. --version
							&option_index)) != -1)
	  {
		if (i == '?' || i != 0)				// invalid arg or arg qualifier so abort
		  {
			return -1;
		  }
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

	ut_check(cef_main(FA_INIT+FA_OPEN, 0) == 0,	// initialise libgxtfa and open clice database
			"Open ce_main.db");
	nc_start();								// initialise libgxtnc and startup ncurses screen display

	while ((iOpt=nc_menu(cpTitle,cpMenu)) != NC_QUIT)		// Display and manage menu until requested to quit
	  {
		switch (iOpt)						// then check for menu selection actions
		  {
			case 1:
				while ((iOpt=nc_menu(	cpTitle+CE_PROJECTS_TITLE_P0,
										cpMenu+CE_PROJECTS_MENU_P0)) != NC_QUIT)	// display and manage menu until requested to quit
				  {
					switch (iOpt)			// then check for menu selection actions
					  {
						case '2':
//							ce_make_all();
							break;
						case '3':
//							ce_make_install();
							break;
					  }
				  }
				break;
			case 2:
			case 3:
			case 4:
				nc_message("Use % for wildcard");
//#TODO Shouldn't tie menu options to the item types held in slice
				CE.iType=iOpt-1;					// set type of ce record required
				i=nc_input(	cpTitle+(CE_PROG_TITLE_P0+((iOpt-2)*2)),
							CE.sName,
							CE_NAME_S0-1);			// set name to look for in clice db

				CE.bmField=CEF_ID_B0+CEF_NAME_B0;	// What to read from the ce database
				CEL.bmField=0;
				ut_check(cef_main(FA_READ+FA_KEY5, 0) == FA_OK_IV0,	// prepare a select of all matching entries
								"Read key5");						// jump to error: if SQL prepare fails.
				int iHits=0;
				while (cef_main(FA_STEP, 0) == FA_OK_IV0)
				  {
					cpList[iHits]=sList[iHits];				// establish an array of pointers for nc_menu
					iList[iHits]=CE.iNo;
					sprintf(sList[iHits++],"%s",CE.sName);	// pointers to the name of each matching record

					if (iHits == CE_LIST_M0)
					  {
						iHits--;
						nc_message("Too many matches - showing first page only");
						sleep(2);
						continue;
					  }
				  }

				if (iHits == 0)							// no matches in clice database
				  {
					nc_message("No matches found");
					sleep(2);
				  }
				else
				  {
					cpList[iHits]=(char *) NULL;		// mark end of search results list

					iPos=(iHits == 1) ? 1 :
						nc_menu(cpTitle+CE_SELECT_TITLE_P0,
								cpList);					// display search results until selection or quit requested
					if (iPos == NC_QUIT) iOpt=NC_QUIT;		// Pass on a quit request from the search results list

					while (iOpt != NC_QUIT)
					  {
						CE.iNo=iList[iPos-1];				// Read more about the selected item
						CE.bmField=CEF_DESC_B0+CEF_LANG_B0;	// What to read from the ce database
						CEL.bmField=0;
						ut_check(	cef_main(FA_READ+FA_STEP, 0) == FA_OK_IV0,		// prepare a select for selected item
									"Read key0");							// jump to error: if SQL prepare fails.
						cpDisp[0]=sDisp[0];									// Display a single item
						sprintf(sDisp[0]," ");
						cpDisp[1]=sDisp[1];
						sprintf(sDisp[1],"Name=%s Language=%c",
								sList[iPos-1],
								CE.cLang);
						cpDisp[2]=sDisp[2];
						sprintf(sDisp[2],"%s",CE.sDesc);
						cpDisp[3]=sDisp[3];
						sprintf(sDisp[3]," ");
						cpDisp[4]=sDisp[4];
						cpDisp[4]=(char *) NULL;		// mark end of display

						iOpt=nc_menu(	cpDisp,
										cpMenu+CE_ACTIONS_MENU_P0);

						switch (iOpt)				// then check for menu selection actions
						  {
							case 1:					// Next item
								if (iPos < iHits) iPos++;
								break;

							case 2:					// Previous item
								if (iPos-1 > 0) iPos--;
								break;

							case NC_QUIT:			// Quit item display so
								if (iHits > 1)			// if only 1 hit then quit to main menu else
								  {
									iPos=nc_menu(	cpTitle+CE_SELECT_TITLE_P0,
													cpList);		// display search results until selection or quit request
									if (iPos != NC_QUIT) iOpt=1;	// Pass on a quit request from the search results list
//#TODO if returning to the search results list then it would be nice to retain our current position rather than return to the top
								  }
								break;
						  }
					  }
				  }
				break;
		  }
	  }

error:							// tidily stop
	nc_stop();					// end ncurses
	cef_main(FA_CLOSE, 0);

	return 0;
  }
