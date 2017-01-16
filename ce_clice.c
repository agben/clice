//--------------------------------------------------------------
//
//	clice - command line coding environment (CE) management menu
//
//	usage:	./ce_clice	or define as the command 'clice'
//
//	GNU GPLv3+ licence	clice by Andrew Bennington Jan-2016 <www.benningtons.net>
//
//--------------------------------------------------------------

#include <getopt.h>		// for getopt_long - argument parsing
#include <ctype.h>		// common I/O functions like toupper()
#include <stdlib.h>		// memory management
#include <string.h>		// for memcpy

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
						"Enter header name",
			(char *) NULL,
						"Enter system routine name",
			(char *) NULL,
						"Select required entry",
			(char *) NULL};

#define CE_PROJECTS_MENU_P0	6		// offsets for each list of menu options
#define CE_ACTIONS_MENU_P0	9
char *cpMenu[] =	{	"1) My Projects",
						"2) My Programs",
						"3) My Header files",
						"!4) System Functions",
						"!5) System Headers",
			(char *) NULL,
						"!1) Make all",
						"!2) Make install",
			(char *) NULL,
						"1) Next",
						"2) Previous",
						"3) Links to",
						"4) Links from",
						"!5) Edit",
						"6) Remove",
						"!7) Make",
			(char *) NULL};


int main(int argc, char **argv)
  {
	static struct option long_options[] = { 		// valid arguments: name, has_arg(yes=1, no=0, opt=2), flag, val
					{"help",	0,	0,	0},			// 0	Keep this order for parsing after getopt_long
					{"version",	0,	0,	0},			// 1
					{NULL,		0,	NULL,	0}
	};

	int option_index = 0;
	int	i, j;
	char *cp;
	int	ios;					// io status
	int	iOpt;					// selected menu option
	int	iPick;					// selected list option
	int iPos;					// position within a search list

								// The following variables hold a master list [0] and a proposed master list [1]
								// i.e. We have an established list of modules but may decide to switch that for the proposed list
	int iHits[2];				// number of hits found by each search

//#TODO	should use malloc for these?
    int iList[CE_LIST_M0][2];					// list of selected clice db items
    char sList[CE_LIST_M0][2][CE_NAME_S0+20];	// list of their names
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
	nc_start();									// initialise libgxtnc and startup ncurses screen display

	while ((iOpt=nc_menu(cpTitle,cpMenu)) != NC_QUIT)	// Display and manage menu until requested to quit
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
				nc_message("Use % for wildcard");
//#TODO Shouldn't tie menu options to the item types held in slice
				CE.iType=(iOpt == 2) ? CE_PROG_T0 : CE_HEAD_T0;	// set type of item required (program, header, ...)
				i=nc_input(	cpTitle+(CE_PROG_TITLE_P0+((iOpt-2)*2)),
							CE.sName,
							CE_NAME_S0-1);			// set name to look for in clice db

				CE.bmField=CEF_ID_B0+CEF_NAME_B0;	// What to read from the ce database
				CEL.bmField=0;
				ut_check(cef_main(FA_READ+FA_KEY5, 0) == FA_OK_IV0,	// prepare a select of all matching entries
								"Read key5");						// jump to error: if SQL prepare fails.
				iHits[0]=0;
				while (cef_main(FA_STEP, 0) == FA_OK_IV0)
				  {
					cpList[iHits[0]]=sList[iHits[0]][0];	// establish an array of pointers for nc_menu - #TODO must be a better way?
					iList[iHits[0]][0]=CE.iNo;
					sprintf(sList[iHits[0]++][0],"%s",CE.sName);	// pointers to the name of each matching record

					if (iHits[0] == CE_LIST_M0)
					  {
						iHits[0]--;
						nc_message("Too many matches - showing first page only");
						sleep(2);
						continue;
					  }
				  }

				if (iHits[0] == 0)							// no matches in clice database
				  {
					nc_message("No matches found");
					sleep(2);
				  }
				else
				  {
					cpList[iHits[0]]=(char *) NULL;		// mark end of search results list

					iPos=(iHits[0] == 1) ? 1 :
						nc_menu(cpTitle+CE_SELECT_TITLE_P0,
								cpList);					// display search results until selection or quit requested
					if (iPos == NC_QUIT) iOpt=NC_QUIT;		// Pass on a quit request from the search results list

					while (iOpt != NC_QUIT)
					  {
						CE.iNo=iList[iPos-1][0];			// Read more about the selected item
						CE.bmField=CEF_DESC_B0+CEF_LANG_B0+
									CEF_CODE_B0+CEF_TYPE_B0;	// What to read from the ce database
						CEL.bmField=0;
						ut_check(	cef_main(FA_READ+FA_STEP, 0) == FA_OK_IV0,		// prepare a select for selected item
									"Read key0");							// jump to error: if SQL prepare fails.

															// #TODO there must be a neater way rather than set these pointers for each line?
															//		Probably by having the display list terminated by a null rather than a pointer to null.
						cpDisp[0]=sDisp[0];					// establish an array of pointers for nc_menu
						sprintf(sDisp[0]," ");

						switch (CE.cLang)					// display item type
						  {
							case 'C':
							case 'H':
								sDisp[2][0] = 'C';
								if (CE.iType == CE_PROG_T0)
									sprintf(&sDisp[2][1]," program");
								else if (CE.iType == CE_HEAD_T0)
									sprintf(&sDisp[2][1]," header");
								else
									sprintf(&sDisp[2][1]," unknown");
								break;
							case 'S':
								sprintf(sDisp[2],"Shell script");
								break;
							default:
								sprintf(sDisp[2],"%c",CE.cLang);
						  }
						cpDisp[1]=sDisp[1];
						snprintf(sDisp[1], sizeof(sDisp[0]), "Name=%s    (%s)",
								sList[iPos-1][0],
								sDisp[2]);

						cpDisp[2]=sDisp[2];
						snprintf(sDisp[2], sizeof(sDisp[0]), "%s", CE.sDesc);

						cpDisp[3]=sDisp[3];
						snprintf(sDisp[3], sizeof(sDisp[0]), "%s", CE.sCode);

						cpDisp[4]=(char *) NULL;		// mark end of display

						iOpt=nc_menu(	cpDisp,
										cpMenu+CE_ACTIONS_MENU_P0);

						switch (iOpt)				// then check for menu selection actions
						  {
							case 1:					// Next item
								if (iPos < iHits[0]) iPos++;
								break;

							case 2:					// Previous item
								if (iPos-1 > 0) iPos--;
								break;

							case 3:					// Calls to
							case 4:					// Called from
								CE.bmField=0;		// need link details not module details
								if (iOpt == 3)
								  {
									CEL.iNtype=CE.iType;	// type of current item
									memcpy(	CEL.sName,
											sList[iPos-1][0],
											CE_NAME_S0);
									CEL.bmField=CEF_LINK_CALLS_B0+CEF_LINK_CTYPE_B0;	// read list of items linked from this one
									ut_check(cef_main(FA_READ,
										"cl.name = % AND cl.ntype = % ORDER BY cl.calls ASC") == FA_OK_IV0,	// prepare a select of all modules linked
										"Read links to");									// to or jump to error: if fails.
								  }
								else
								  {
									CEL.iCtype=CE.iType;	// type of current item
									memcpy(	CEL.sCalls,
											sList[iPos-1][0],
											CE_NAME_S0);
									CEL.bmField=CEF_LINK_NAME_B0+CEF_LINK_NTYPE_B0;		// read list of items that link to this one
									ut_check(cef_main(FA_READ,
										"cl.calls = % AND cl.ctype = % ORDER BY cl.name ASC") == FA_OK_IV0,	// prepare a select of all modules linked
										"Read links from");									// from or jump to error: if fails.
								  }

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

									sprintf(sList[iHits[1]][1], " %-*s   %s",
											CE_NAME_S0,
											cp,
											(i == CE_PROG_T0) ?	"function":
											(i == CE_HEAD_T0) ?	"header":
																"unknown");
									iList[iHits[1]++][1]=0;		// Will determine if each called item exists in clice later
									if (iHits[1] == CE_LIST_M0)		//#TODO need to handle large lists better - this will keep pausing
									  {
										iHits[1]--;
										nc_message("Too many matches - showing first page only");
										sleep(2);
										continue;
									  }
								  }

								if (iHits[1] == 0)						// no matches in clice database
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

									for (i=0; i < iHits[1]; i++)
									  {
										for(j=0; sList[i][1][j+1] > ' '; j++)
											CE.sName[j]=sList[i][1][j+1];
										CE.sName[j]='\0';			// extract item name and null terminate

										CE.bmField=CEF_ID_B0;		// See which called modules exist in clice
										CEL.bmField=0;
										ut_check(cef_main(FA_READ+FA_KEY1,
															0) == FA_OK_IV0,	// prepare a select for selected item
												"Read key1");		// jump to error: if SQL prepare fails.

										if (cef_main(FA_STEP,0) == FA_OK_IV0)
											iList[i][1]=CE.iNo;
										else
											sList[i][1][0]='!';		// Mark module as unknown in clice
									  }

									iPick=nc_menu(	cpTitle+CE_SELECT_TITLE_P0,
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
														CE_NAME_S0);
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
