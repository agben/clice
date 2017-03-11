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

//--------------------------------------------------------------
//	Manage projects with clice
//
//	ce_clice_project(int iProjCount)
//		where iProjCount = count of projects to list
//--------------------------------------------------------------

#define	CE_PROJ_LIST_S0 (CE_PROJECT_S0+3+CE_DESC_S0)	// width of project list

void ce_clice_project(int iProjCount)
  {
	int	i;
	int	iOpt;						// selected menu option
	int iPos;						// position within a search list
	char sBuff[CE_PROJ_LIST_S0+10];	// string buffer

	char *cpProjMenu[]=	{			// Project menu template
					"1) Next",
					"2) Previous",
					"!3) Make all",
					"!4) Make install",
					NULL};
	char **cpList;					// pointers to a list of projects to list
	char *cp;						// pointer to memory for project name list


	ut_check((cpList = malloc((sizeof(char *)*iProjCount)+1)) != NULL &&
			 (cp = malloc(iProjCount*CE_PROJ_LIST_S0)) != NULL,
			"malloc error");

	CE.bmField=CEF_PROJECT_B0+CEF_DESC_B0;		// Read all projects in the clice db
	CEL.bmField=0;
	CE.iType=CE_PROJ_T0;
	ut_check(cef_main(FA_READ,"ce.type = % ORDER BY ce.project") == FA_OK_IV0,
		"read projects");
	for (i=0; i < iProjCount && cef_main(FA_STEP, 0) == FA_OK_IV0; i++)
	  {
		cpList[i]=&cp[i*CE_PROJ_LIST_S0];
		snprintf(	cpList[i],
					CE_PROJ_LIST_S0,
					"%s   %s",
					CE.sProject, CE.sDesc);
	  }
	cpList[i]=NULL;								// mark end of projects list

	iOpt=iPos=(iProjCount == 1) ? 1 :
			nc_menu("Select required entry",
					cpList);					// display project list until selection or quit requested
	while (iOpt != NC_QUIT)
	  {
		snprintf(	sBuff,
					sizeof(sBuff),
					"Project:%s",
					cpList[iPos-1]);			// display selected project

		iOpt=nc_menu(sBuff, cpProjMenu);		// menu of project actions
		switch (iOpt)
		  {
			case 1:								// Next item
				if (iPos < iProjCount) iPos++;
				break;
			case 2:								// Previous item
				if (iPos-1 > 0) iPos--;
				break;
			case 3:
//				ce_make_all();
				break;
			case 4:
//				ce_make_install();
				break;
		  }
	  }
error:
	free(cpList);
	free(cp);
  };


//--------------------------------------------------------------
//
//	Manage modules (programs & headers) with clice
//
//	ce_clice_module(int iCount)
//		where iCount = count of modules to list
//--------------------------------------------------------------

#define	CE_MOD_LIST_S0	(CE_NAME_S0+20)		// width of module list
#define CE_DISP_M0	3						// Header lines in a module display
#define CE_MOD_MENU_M0	7					// enties in the module memory

void ce_clice_module(int iCount)
  {
	int	i, j;
	char *cp;
	int	ios;					// io status
	int	iOpt = -1;				// selected menu option
	int	iPick;					// selected list option
	int iPos;					// position within a search list
	int iHits;					// number of search hits
	int iCountTo=0;				// count of links to other modules
	int iCountFrom=0;			// count of links from other modules

	char sBuff[CE_DISP_M0*CE_DESC_S0];			// string buffer
	char sType[20];				// type of module
	char cMenu[2][30];			// modified menu options

	char *cpMenu[CE_MOD_MENU_M0+1];
	char *cpModMenu[]=	{		// Module menu template
			"1) Next",
			"2) Previous",
			"3) Links to",
			"4) Linked from",
			"!5) Edit",
			"6) Remove",
			"!7) Make"};

	char **cpList;
	char *cp1;					// List of selected module names
	char *cp2;
	int *ip1;					//	and their id's
	int *ip2;

	ut_check((cpList = malloc((sizeof(char *)*iCount)+1)) != NULL &&
			 (cp1 = malloc(iCount*CE_MOD_LIST_S0)) != NULL &&
			 (cp2 = malloc(0)) != NULL &&
			 (ip1 = malloc(sizeof(int)*iCount)) != NULL &&
			 (ip2 = malloc(0)) != NULL,
			"malloc error");

	CE.bmField=CEF_ID_B0+CEF_NAME_B0;			// What to read from the ce database
	CEL.bmField=0;
	ut_check(cef_main(FA_READ+FA_KEY5,0) == FA_OK_IV0,
			"Read key5");						// jump to error: if SQL prepare fails.

	for (i=0; i < iCount && cef_main(FA_STEP, 0) == FA_OK_IV0; i++)
	  {
		cpList[i]=&cp1[i*CE_MOD_LIST_S0];		// establish an array of pointers for nc_menu
		sprintf(cpList[i],"%s",CE.sName);		// pointers to the name of each matching record
		ip1[i]=CE.iNo;
	  }
	cpList[i]=NULL;

	iOpt=iPos=(iCount == 1) ? 1 :
			nc_menu("Select required entry",
					cpList);					// display search results until selection or quit requested
	while (iOpt != NC_QUIT)
	  {
		CE.iNo=ip1[iPos-1];						// Read more about the selected item
		CE.bmField=CEF_DESC_B0+CEF_LANG_B0+
					CEF_CODE_B0+CEF_TYPE_B0;	// What to read from the ce database
		CEL.bmField=0;
		ut_check(cef_main(FA_READ+FA_STEP,
							0) == FA_OK_IV0,	// prepare a select for selected item
						"Read key0");			// jump to error: if SQL prepare fails.


		switch (CE.iType)						// display item type
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

		for (i=0; cp1[((iPos-1)*CE_MOD_LIST_S0)+i] > ' ' && i < CE_NAME_S0; i++);	//Find length of item name
		j=snprintf(sBuff, sizeof(sBuff), "%-*.*s    (%s)\n",
				i, i,
				&cp1[(iPos-1)*CE_MOD_LIST_S0],
				sType);

		if (CE.iType < CE_SYSF_T0)			// local function/header?
			j+=snprintf(&sBuff[j], sizeof(sBuff)-j, "%s", CE.sDesc);
		sBuff[j++]='\n';

		if (CE.iType > CE_HEAD_T0 || CE.cLang == 'H' || CE.cLang == 'S')
			sBuff[j++]='\0';				// no example code to display for header or script files
		else
			j+=snprintf(&sBuff[j], sizeof(sBuff)-j, "%s", CE.sCode);

		CE.bmField=0;						// Count how many items link to/from this module
		CEL.bmField=CEF_LINK_COUNT_B0;
		for (i=0; i < CE_MOD_MENU_M0; i++)
		  {
			if (i == 2 || i == 3)
			  {
				if (i == 2)
				  {
					CEL.iNtype=CE.iType;			// type of current item
					for (j=0; cp1[((iPos-1)*CE_MOD_LIST_S0)+j] > ' '; j++)
							CEL.sName[j]=cp1[((iPos-1)*CE_MOD_LIST_S0)+j];
					CEL.sName[j]='\0';
					ut_check(cef_main(FA_COUNT,
						"cl.name = % AND cl.ntype = %") == FA_OK_IV0,	// prepare a select of all modules linked
						"Count links to");			// to or jump to error: if fails.
					iCountTo=CE.iCount;
				  }
				else
				  {
					CEL.iCtype=CE.iType;			// type of current item
					for (j=0; cp1[((iPos-1)*CE_MOD_LIST_S0)+j] > ' '; j++)
							CEL.sCalls[j]=cp1[((iPos-1)*CE_MOD_LIST_S0)+j];
					CEL.sCalls[j]='\0';
					ut_check(cef_main(FA_COUNT,
						"cl.calls = % AND cl.ctype = %") == FA_OK_IV0,	// prepare a select of all modules linked
						"Count links from");		// from or jump to error: if fails.
					iCountFrom=CE.iCount;
				  }
				j=0;
				if (CE.iCount <= 0) cMenu[i-2][j++]='!';	// No links so disable this option
				j+=sprintf (&cMenu[i-2][j], "%s", *(cpModMenu+i));
				if (CE.iCount > 0) sprintf (&cMenu[i-2][j], " [%d]", CE.iCount);
				cpMenu[i]=cMenu[i-2];
			  }
			else
				cpMenu[i]=cpModMenu[i];
		  }
		cpMenu[i]=NULL;

		iOpt=nc_menu(sBuff, cpMenu);
		switch (iOpt)						// then check for menu selection actions
		  {
			case 1:							// Next item
				if (iPos < iCount) iPos++;
				break;

			case 2:							// Previous item
				if (iPos-1 > 0) iPos--;
				break;

			case 3:							// Calls to
			case 4:							// Called from
				CE.bmField=0;				// need link details not module details
				if (iOpt == 3)
				  {
					CEL.bmField=CEF_LINK_CALLS_B0+CEF_LINK_CTYPE_B0;	// read list of items linked from this one
					ut_check(cef_main(FA_READ,
						"cl.name = % AND cl.ntype = % ORDER BY cl.calls ASC") == FA_OK_IV0,	// prepare a select of all modules linked
						"Read links to");			// to or jump to error: if fails.
					CE.iCount=iCountTo;
				  }
				else
				  {
					CEL.bmField=CEF_LINK_NAME_B0+CEF_LINK_NTYPE_B0;		// read list of items that link to this one
					ut_check(cef_main(FA_READ,
						"cl.calls = % AND cl.ctype = % ORDER BY cl.name ASC") == FA_OK_IV0,	// prepare a select of all modules linked
							"Read links from");			// from or jump to error: if fails.
					CE.iCount=iCountFrom;
				  }

				ut_check((cpList = realloc(cpList, (sizeof(char *)*CE.iCount)+1)) != NULL &&
						 (cp2 = realloc(cp2, CE.iCount*CE_MOD_LIST_S0)) != NULL &&
						 (ip2 = realloc(ip2, sizeof(int)*CE.iCount)) != NULL,
						"realloc2 error");

				int iLen=0;							// Find longest name in list
				int iMyHits=0;						// module counts of my modules, not system modules
				iHits=0;
				while (iHits < CE.iCount && cef_main(FA_STEP, 0) == FA_OK_IV0)
				  {
					cpList[iHits]=&cp2[iHits*CE_MOD_LIST_S0];	// establish an array of pointers for nc_menu
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

					sprintf(&cp2[iHits*CE_MOD_LIST_S0], "%-*s   %s",
							CE_NAME_S0,
							cp,
							(i == CE_PROG_T0) ?	"function":
							(i == CE_HEAD_T0) ?	"header":
							(i == CE_SYSF_T0) ?	"system function":
							(i == CE_SYSH_T0) ?	"system Header":
												"unknown");
					j=strnlen(cp, CE_NAME_S0);
					if (j > iLen) iLen=j;			// check if this is the longest name?

					ip2[iHits]=0;					// Will determine if each called item exists in clice later
					iHits++;
				  }

				cpList[iHits]=NULL;					// mark end of search results list
				iLen+=3;							// allow this many spaces between name and type in the list

				for (i=0; i < iHits; i++)
				  {
					for(j=0; cp2[(i*CE_MOD_LIST_S0)+j] > ' '; j++)
							CE.sName[j]=cp2[(i*CE_MOD_LIST_S0)+j];
					CE.sName[j]='\0';				// extract item name and null terminate

					for (j=iLen; cp2[(i*CE_MOD_LIST_S0)+j] != '\0'; j++)
					cp2[(i*CE_MOD_LIST_S0)+j]=
						cp2[(i*CE_MOD_LIST_S0)+j+(CE_NAME_S0-iLen)];	// reduce blank space in output list

					CE.bmField=CEF_ID_B0;			// See which called modules exist in clice
					CEL.bmField=0;
					ut_check(cef_main(FA_READ+FA_KEY1,		// #TODO should read by name + type
										0) == FA_OK_IV0,	// prepare a select for selected item
							"Read key1");			// jump to error: if SQL prepare fails.

					if (cef_main(FA_STEP,0) == FA_OK_IV0)
					  {
						ip2[i]=CE.iNo;				// module exists in clice db so one of mine and not a system module
						iMyHits++;
					  }
					else							// Mark module as unknown in clice by inserting a ! marker
					  {
						cp2[((i+1)*CE_MOD_LIST_S0)-1]='\0';	// ensure null termination of string
						for(j=CE_MOD_LIST_S0-2; j > 0; j--)
							cp2[(i*CE_MOD_LIST_S0)+j]=cp2[(i*CE_MOD_LIST_S0)+j-1];
						cp2[i*CE_MOD_LIST_S0]='!';
					  }
				  }

				iPick=nc_menu("Select required entry", cpList);	// display search results until selection or quit requested
				if (iPick != NC_QUIT)							// Switch master search list with our proposed list
				  {
					ut_check((cp1 = realloc(cp1, iMyHits*CE_MOD_LIST_S0)) != NULL &&
							 (ip1 = realloc(ip1, sizeof(int)*iMyHits)) != NULL,
						"realloc1 error");

					iCount=0;
					for (i=0; i < iHits; i++)
					  {
						if (cp2[i*CE_MOD_LIST_S0] != '!')		// pick only the entries that are recorded in the clice db
						  {
							ip1[iCount]=ip2[i];
							memcpy(	&cp1[iCount*CE_MOD_LIST_S0],
									&cp2[i*CE_MOD_LIST_S0],
									CE_MOD_LIST_S0);
							iCount++;
							if (i == iPick-1) iPos=iCount;	// position of selected item within the new (clice db entries only) list
						  }
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
				memcpy(CEL.sName,&cp1[(iPos-1)*CE_MOD_LIST_S0],CE_NAME_S0);
				memcpy(CEL.sCalls, CEL.sName, CE_NAME_S0);
				ios=cef_main(FA_DELETE, "cl.name = % OR cl.calls = %");		// 2nd delete all links to+from this item
				ut_check(ios == FA_OK_IV0, "Delete links %d", ios);

				nc_message("Item has been removed from the clice db");
				sleep(2);

				iOpt=NC_QUIT;				// change action to ensure menus behave

				break;

			case NC_QUIT:					// Quit item display
				if (iCount > 1)			// if only 1 hit then quit to main menu else
				  {
					iPos=nc_menu("Select required entry",
									cpList);		// display search results until selection or quit request
					if (iPos != NC_QUIT) iOpt=1;	// Pass on a quit request from the search results list
//#TODO if returning to the search results list then it would be nice to retain our current position rather than return to the top
				  }
				break;
		  }
	  }

error:
	free(cpList);
	free(cp1);
	free(cp2);
	free(ip1);
	free(ip2);
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
	int	iProjCount;				// project count

	char *cpMainMenu[]=	{		// main menu template
					"1) My Projects",
					"2) My Programs",
					"3) My Header files",
					"4) System Functions",
					"5) System Headers",
			NULL};

	char cMenu[CE_SYSH_T0+1][30];	// currently one menu item per module type with system headers the last (SYSH)
    char *cpList[CE_SYSH_T0+2];		// +2 to allow for a null pointer to terminate the menu list


	if (ce_args(argc, argv) < 0) goto error;	// process any command arguments i.e. clice --version

	ut_check(cef_main(FA_INIT+FA_OPEN, 0) == 0,
			"Open ce_main.db");					// initialise libgxtfa and open clice database

	nc_start();									// initialise libgxtnc and startup ncurses screen display

	while (iOpt != NC_QUIT)						// Display and manage menu until requested to quit
	  {
		CE.bmField=CEF_COUNT_B0;
		CEL.bmField=0;
		for (CE.iType=CE_PROJ_T0; CE.iType <= CE_SYSH_T0; CE.iType++)
		  {
			cef_main(FA_COUNT, "ce.type = %");	// Create menu with up to date counts of module types
			i=0;
			if (CE.iCount <= 0) cMenu[CE.iType][i++]='!';	// No items so disable this option
			i+=sprintf (&cMenu[CE.iType][i], "%s", *(cpMainMenu+CE.iType));
			if (CE.iCount > 0) sprintf (&cMenu[CE.iType][i], " [%d]", CE.iCount);
			cpList[CE.iType]=cMenu[CE.iType];
			if (CE.iType == CE_PROJ_T0) iProjCount=CE.iCount;
		  }
		cpList[CE.iType]=NULL;

		switch ((iOpt=nc_menu("clice Menu",cpList)))
		  {
			case 1:
				ce_clice_project(iProjCount);
				break;
			case 2:
			case 3:
			case 4:
			case 5:
				CE.iType=iOpt-1;			// set type of item required (program, header, ...)

				nc_message("Use % for wildcard");
				nc_input(	"Select required module",
							CE.sName,
							CE_NAME_S0-1);					// input name to look for in clice db
				i=strnlen(CE.sName, CE_NAME_S0-1);
				if (i < CE_NAME_S0-1) CE.sName[i++]='%';	// if room add a default wildcard
				CE.sName[i]='\0';

				CE.bmField=CEF_COUNT_B0;
				CEL.bmField=0;
				cef_main(FA_COUNT,
						"ce.name LIKE % AND ce.type = %");	// count matching modules

				if (CE.iCount <= 0)							// no matches in clice database
				  {
					nc_message("No matches found");
					sleep(2);
				  }
				else
					ce_clice_module(CE.iCount);
				break;
		  }
	  }

error:							// tidily stop
	nc_stop();					// end ncurses
	cef_main(FA_CLOSE, 0);

	return 0;
  }
