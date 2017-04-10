//--------------------------------------------------------------
//
// Generate a makefile for a project based on clice data
//
//	usage:	$ce_gen_make
//		The current directory is used to determine which project makefile to generate.
//
//		options
//			--help			= basic help
//			--project proj	= provide project name - else default to using project in the current directory
//			--version		= version, copyright and licence details
//
//		${GXT_CODE_HOME}/.makefile.template is used to generate a fresh makefile with #~ markers replaced by this program
//			with project data from the clice database.
//
//	GNU GPLv3+ licence	clice - The command-line coding ecosystem by Andrew Bennington 2017 <www.benningtons.net>
//
//--------------------------------------------------------------

#include <ctype.h>		// for tolower
#include <fcntl.h>		// file access functions
#include <stdlib.h>		// for getenv and memory management
#include <string.h>		// for strcopy

#include <fa_def.h>		// file actions
#include <ce_main.h>	// CE database
#include <ut_date.h>	// date and time utilities
#include <ut_error.h>	// for debug and checks

#define	BUFF_S0 200		// Output buffer size


struct CE_EXTRACT
  {
	int		iType;				// module type i.e. function or header
	char	sName[CE_NAME_S0];	// module name
	char	cMain;				// executable program?
	char	cLang;				// language used
  };


//--------------------------------------------------------------
//
// When extracting a list of modules first check if already extracted. If not then add to list.
//
// Arguments:
//	sp		- pointer to the structure list of extracted modules
//	i		- current number in the list
//	iMax	- max list size - if exceeded then allocate more memory
//	sName	- pointer to module name that could be added to the sp list
//	iType	- type of the module sName
//
//--------------------------------------------------------------

int ce_gen_make_add(struct CE_EXTRACT **sp, int *i, int *iMax, const char *sName, const int iType)
  {
	int j;

	for (j=0; j < *i; j++)
	  {
		if ((*sp+j)->iType == iType &&
			strncmp((*sp+j)->sName, sName, CE_NAME_S0) == 0) break;
	  }
	if (j >= *i)									// new item to add to recursive list
	  {
		(*sp+(*i))->iType=iType;
		memcpy(	(*sp+(*i))->sName,
				sName,
				CE_NAME_S0);
		if (++*i == *iMax)						// need more memory to hold list
		  {
			*iMax+=50;
			ut_check((*sp = realloc(*sp, sizeof(struct CE_EXTRACT)*(*iMax))) != NULL,
					"realloc error");
		  }
	  }
	return j;
error:
	return -1;
  }


//--------------------------------------------------------------
//
// Find the source file for the passed function and add it to the passed list
//
// Arguments:
//	sp		- pointer to the structure list of extracted modules
//	i		- current number in the list
//	iMax	- max list size - if exceeded then allocate more memory
//	sName	- pointer to module name that could be added to the sp list
//	iType	- type of the module sName
//
// Returns:
//	-1	- error
//	0	- source already exists in list
//	1	- source added to list
//
//--------------------------------------------------------------

int ce_gen_make_source(struct CE_EXTRACT **sp, int *i, int *iMax, const char *sName, const int iType)
  {
	CE.bmField=CEF_SOURCE_B0;
	CEL.bmField=0;
	memcpy(	CE.sName,
			sName,
			CE_NAME_S0);
	CE.iType=iType;
	if (cef_main(FA_READ+FA_STEP,"ce.name = % AND ce.type = %") != FA_OK_IV0)
		ut_error("no source found for %s", CE.sName);

	int j=*i;
	ut_check(ce_gen_make_add(sp, i, iMax, CE.sSource, CE_SRCE_T0) >= 0,
			"add");
	if (*i > j) return 1;						// source not already listed

	return 0;
error:
	return -1;
  }


//--------------------------------------------------------------
//
// Output dependency and compilation commands
//
// Arguments:
//	sp		- pointer to root program that needs a dependency list and compilation command output to makefile
//	fp		- file pointer to new makefile
//
//--------------------------------------------------------------

void ce_gen_make_out(struct CE_EXTRACT **sp, FILE *fp)
  {
	struct CE_EXTRACT *spC;	// list of modules linked to the extracted program, sp
	int iC = 0;				// count of modules linked
	int iCmax = 50;			// initial memory allocation for these module lists
	int iDirect = 0;		// count of how many modules are directly linked from the main program
	int ios = -1;			// i/o status
	char sBuff[BUFF_S0];	// output buffer for building makefile lines


	ut_check((spC = malloc(sizeof(struct CE_EXTRACT)*iCmax)) != NULL,	// initial allocation for lists of called modules
			"malloc error");

	CE.bmField=0;
	CEL.bmField=CEF_LINK_CALLS_B0+CEF_LINK_CTYPE_B0;

	memcpy(	spC->sName,
			(*sp)->sName,
			CE_NAME_S0);						// start by checking everything linked to the passed program
	spC->iType=CE_PROG_T0;

	for (int j=0; j <= iC; j++)					// build a recursive list of called modules
	  {
		memcpy(	CEL.sName,
				(spC+j)->sName,
				CE_NAME_S0);
		CEL.iNtype=(spC+j)->iType;
		ut_check(cef_main(FA_READ,"cl.name = % AND cl.ntype = % ORDER BY cl.calls ASC") == FA_OK_IV0,
					"read");

		while(cef_main(FA_STEP, 0) == FA_OK_IV0)
			if (strncmp(CEL.sCalls, (*sp)->sName, CE_NAME_S0) != 0)		// ignore any recursive calls to same function
				ut_check(ce_gen_make_add(&spC, &iC, &iCmax, CEL.sCalls, CEL.iCtype) >= 0,
						"add");
		if (j == 0) iDirect=iC;					// end of direct links, all after iDirect are recursive
	  }

	char *cp = &sBuff[0];						// generate makefile compliation lists
	if ((*sp)->cMain == CE_MAIN_T0)				// If an executable then find run-time libraries to link with
	  {
		CE.bmField=0;
		CEL.bmField=CEF_LINK_CALLS_B0;
		for (int j=0; j <= iC; j++)				// add any run-time libraries to the list
		  {
			if ((spC+j)->iType == CE_SYSH_T0)	// run-time libraries are associated with system headers in <project>.clice
			  {
				memcpy(	CEL.sName,
						(spC+j)->sName,
						CE_NAME_S0);
				CEL.iNtype=CE_SYSH_T0;
				CEL.iCtype=CE_RLIB_T0;
				ut_check(cef_main(FA_READ,"cl.name = % AND cl.ntype = % AND cl.ctype = %") == FA_OK_IV0,
							"read");

				while(cef_main(FA_STEP, 0) == FA_OK_IV0)
					ut_check(ce_gen_make_add(&spC, &iC, &iCmax, CEL.sCalls, CE_RLIB_T0) >= 0,
							"add");
			  }
		  }

		cp+=snprintf(cp, BUFF_S0, "%s: %s.%c",
			(*sp)->sName, (*sp)->sName, tolower((*sp)->cLang));

		iDirect=iC;								// executable so needs to link with all recursive routines
	  }
	else
	  {
		cp+=snprintf(cp, BUFF_S0, "$(objdir)/%s.o: %s.%c",		//#TODO will need to check if source file named differently
			(*sp)->sName, (*sp)->sName, tolower((*sp)->cLang));
	  }
	ce_gen_make_source(&spC, &iC, &iCmax, (*sp)->sName, (*sp)->iType);

	for (int j=0; j < iDirect; j++)					// start at 1 because the 1st is the main program
	  {
		if ((spC+j)->iType == CE_HEAD_T0)
			cp+=snprintf(cp, BUFF_S0-(cp-&sBuff[0]),
					" $(includedir)/%s.h", (spC+j)->sName);
		else if ((spC+j)->iType == CE_PROG_T0)
		  {
			ios=ce_gen_make_source(&spC, &iC, &iCmax, (spC+j)->sName, (spC+j)->iType);
			if (ios == 1)					// source not already listed so use it?
			  {
				int i;
				CE.bmField=0;			// first check if the object file is in a library
				CEL.bmField=CEF_LINK_CALLS_B0;
				memcpy(	CEL.sName,
						CE.sSource,
						CE_NAME_S0);
				CEL.iNtype=CE_OBJT_T0;
				CEL.iCtype=CE_OLIB_T0;
				for (i=0; i < CE_NAME_S0 && CEL.sName[i] != '.'; i++);
				if (i >= CE_NAME_S0-1) ut_error("no file ext: %s", CEL.sName);
				sprintf(&CEL.sName[i+1], "o");

				if (cef_main(FA_READ+FA_STEP,
						"cl.name = % AND cl.ntype = % AND cl.ctype = %") == FA_OK_IV0)
				  {
					i=iC;
					ut_check(ce_gen_make_add(&spC, &iC, &iCmax, CEL.sCalls, CE_OLIB_T0) >= 0,
							"add");
					if (iC > i)					// object library not already listed so use it?
						cp+=snprintf(cp, BUFF_S0-(cp-&sBuff[0]), " $(objdir)/%s", CEL.sCalls);
				  }
				else
					cp+=snprintf(cp, BUFF_S0-(cp-&sBuff[0]), " $(objdir)/%s", CEL.sName);
			  }
		  }

		if ((cp-&sBuff[0]) > 70)					// adjust to alter line length in the makefile
		  {
			snprintf(cp, BUFF_S0-(cp-&sBuff[0]), " \\\n");
			ut_check((ios=fputs(sBuff, fp)), "write %d", ios);
			cp = &sBuff[0]+1;
			sBuff[0]='\t';
		  }
	  }

	cp+=snprintf(cp, BUFF_S0, " \n\t$(GCC) $(CFLAGS) ");	// #TODO defaulting to C

	if ((*sp)->cMain == CE_MAIN_T0)				// If an executable then find run0time libraries to link with
	  {
		cp+=snprintf(cp, BUFF_S0, "$^ -o $@");
		for (int j=0; j <= iC; j++)							// link with any run-time libraries?
			if ((spC+j)->iType == CE_RLIB_T0)
				cp+=snprintf(cp, BUFF_S0-(cp-&sBuff[0]), " -l%s", (spC+j)->sName);
	  }
	else
		cp+=snprintf(cp, BUFF_S0, "-c $< -o $@");

	cp+=snprintf(cp, BUFF_S0-(cp-&sBuff[0]), "\n");

	ut_check((ios=fputs(sBuff, fp)), "write %d", ios);

error:
	if (spC != NULL) free(spC);
	return;
  }





int main(int argc, char **argv)
  {
						// spE, iE, iEmax = extracted clice db entries for selected project
						// spC, iC, iCmax = called functions and included headers
	struct CE_EXTRACT *spE, *sp;
	int iE = 0;				// count of modules for selected project
	int iEmax = 50;


	FILE *fpT = NULL,		// file pointer for template
		 *fpN = NULL;		// file pointer for new makefile
	char sBuff[BUFF_S0];	// input buffer for template reads
	char *cp;
	int i;
	int ios = -1;			// i/o status


	spCE = (struct CE_FIELDS*) &CE;
	spCEL = (struct CEL_FIELDS*) &CEL;

	CE.sProject[0]='\0';						// clear project to see if --project is picked up by ce_args
	if (ce_args(argc, argv) < 0) goto error;
	if (CE.sProject[0] == '\0')					// no project name provided with --project argument so use current directory
	  {
		ut_check(getcwd(CE.sDir, sizeof(CE.sDir)), "getcwd");
		for (i=0; i < CE_PROJECT_S0; i++)
			CE.sProject[i]=CE.sDir[strlen(CE.sDir)+i+1-CE_PROJECT_S0];
		CE.sProject[i]='\0';
	  }

	ut_check((ios=cef_main(FA_INIT+FA_OPEN, 0)) == 0, // Initialise libgxtfa and open clice db
		"Open clice db error %d", ios);

	sprintf(sBuff,
			"%s/.makefile.template",
			getenv("GXT_CODE_HOME"));			// open the clice makefile template
	ut_check((fpT=fopen(sBuff, "r")) != NULL, "open template");

	ut_check((fpN=fopen("makefile", "w")) != NULL, "open new makefile");

	CE.bmField=CEF_TYPE_B0+CEF_NAME_B0+
					CEF_LANG_B0+CEF_MAIN_B0;
	CEL.bmField=0;
	CE.iType=CE_SYSF_T0;
	ut_check(cef_main(FA_READ,"ce.project = % AND ce.type < % ORDER BY ce.type DESC, ce.name ASC") == FA_OK_IV0,
					"read project");

	while (fgets(sBuff, BUFF_S0, fpT) != NULL)		// read through template looking for clice markers '#~'
	  {
		if (sBuff[0] != '#' || sBuff[1] != '~')		// non-marker lines to be written directly to generated file
		  {
			ut_check((ios=fputs(sBuff, fpN)), "write %d", ios);
			continue;
		  }

		if (memcmp(&sBuff[2], "bui", 3) == 0)		// build date stamp
		  {
			ut_date_now();							// #TODO improve date and time functions
			snprintf(sBuff, BUFF_S0, "# built on %.2d%.2d%d at %.4d\n",
					gxt_iDate[0]%100, (gxt_iDate[0]%10000)/100,
					gxt_iDate[0]/10000, gxt_iTime[0]/100);
			ut_check((ios=fputs(sBuff, fpN)), "write %d", ios);
			continue;
		  }

		ut_check(cef_main(FA_RESET, 0) == FA_OK_IV0, "reset");	// reset prepare to step through project list again

		if (memcmp(&sBuff[2], "exe", 3) == 0)					// list project executables
		  {
			cp = &sBuff[0]+1;
			sBuff[0]='\t';

			while(cef_main(FA_STEP, 0) == FA_OK_IV0)
			  {
				if (CE.iType == CE_PROG_T0 && CE.cMain == CE_MAIN_T0)
				  {
					cp+=snprintf(cp, BUFF_S0-(cp-&sBuff[0]), "%s ", CE.sName);

					if ((cp-&sBuff[0]) > 70)					// adjust to alter line length in the makefile
					  {
						snprintf(cp, BUFF_S0-(cp-&sBuff[0]), "\\\n");
						ut_check((ios=fputs(sBuff, fpN)), "write %d", ios);
						cp = &sBuff[0]+1;
						sBuff[0]='\t';
					  }
				  }
			  }
			snprintf(cp, BUFF_S0-(cp-&sBuff[0]), "\n");			// output remainder of last line or a blank continuation
			ut_check((ios=fputs(sBuff, fpN)), "write %d", ios);
		  }
		else if (memcmp(&sBuff[2], "fun", 3) == 0)				// extract a list of project functions to compile
		  {
			ut_check((spE = malloc(sizeof(struct CE_EXTRACT)*iEmax)) != NULL,
						"malloc error");

			while(cef_main(FA_STEP, 0) == FA_OK_IV0)
			  {
				if (CE.iType == CE_PROG_T0 && CE.cLang != 'S')
				  {
					ut_check(ce_gen_make_add(&spE, &iE, &iEmax, CE.sName, CE.iType) >= 0,
						"add");
					(spE+iE-1)->cLang=CE.cLang;
					(spE+iE-1)->cMain=CE.cMain;
				  }
			  }
		  }
		else if (memcmp(&sBuff[2], "ins", 3) == 0)	// install the project
		  {
			cp = &sBuff[0]+1;
			sBuff[0]='\t';

			while(cef_main(FA_STEP, 0) == FA_OK_IV0)
			  {
				if (CE.iType == CE_PROJ_T0) continue;
				if (CE.iType == CE_HEAD_T0)
					cp+=snprintf(cp, BUFF_S0-(cp-&sBuff[0]), "$(includedir)/%s.%c ", CE.sName, tolower(CE.cLang));
				else if (CE.cLang == 'S')
				  {
					if (strstr(CE.sName, "complete") > 0)	// script names including 'complete' assumed to be bash completion
						cp+=snprintf(cp, BUFF_S0-(cp-&sBuff[0]), "$(completedir)/%s.sh ", CE.sName);
					else
						cp+=snprintf(cp, BUFF_S0-(cp-&sBuff[0]), "$(bindir)/%s.sh ", CE.sName);
				  }
				else if (CE.iType == CE_PROG_T0 && CE.cMain == CE_MAIN_T0)
					cp+=snprintf(cp, BUFF_S0-(cp-&sBuff[0]), "$(bindir)/%s ", CE.sName);

				if ((cp-&sBuff[0]) > 70)						// adjust to alter line length in the makefile
				  {
					snprintf(cp, BUFF_S0-(cp-&sBuff[0]), "\\\n");
					ut_check((ios=fputs(sBuff, fpN)), "write %d", ios);
					cp = &sBuff[0]+1;
					sBuff[0]='\t';
				  }
			  }
			snprintf(cp, BUFF_S0-(cp-&sBuff[0]), "\n");			// output remainder of last line or a blank continuation
			ut_check((ios=fputs(sBuff, fpN)), "write %d", ios);

			ut_check(cef_main(FA_RESET, 0) == FA_OK_IV0, "reset");

			while(cef_main(FA_STEP, 0) == FA_OK_IV0)			// step through project files again to create install dependencies
			  {
				if (CE.iType == CE_PROJ_T0) continue;

				cp = &sBuff[0];
				if (CE.iType == CE_HEAD_T0)
					cp+=snprintf(cp, BUFF_S0, "$(includedir)/%s.%c: %s.%c\n",
							CE.sName, tolower(CE.cLang), CE.sName, tolower(CE.cLang));
				else if (CE.cLang == 'S')
				  {
					if (strstr(CE.sName, "complete") > 0)	// script names including 'complete' assumed to be bash completion
						cp+=snprintf(cp, BUFF_S0, "$(completedir)/%s.sh: %s.sh\n", CE.sName, CE.sName);
					else
						cp+=snprintf(cp, BUFF_S0, "$(bindir)/%s.sh: %s.sh\n", CE.sName, CE.sName);
				  }
				else if (CE.iType == CE_PROG_T0 && CE.cMain == CE_MAIN_T0)
					cp+=snprintf(cp, BUFF_S0, "$(bindir)/%s: %s\n", CE.sName, CE.sName);

				if (cp > sBuff)
				  {
					snprintf(cp, BUFF_S0, "\tsudo cp $^ $@\n");
					ut_check((ios=fputs(sBuff, fpN)), "write %d", ios);
				  }
			  }
		  }
		else if (memcmp(&sBuff[2], "uni", 3) == 0)	// uninstall the project
		  {
			while(cef_main(FA_STEP, 0) == FA_OK_IV0)
			  {
				if (CE.iType == CE_PROJ_T0) continue;

				sBuff[0]=0;
				if (CE.iType == CE_HEAD_T0)
					snprintf(sBuff, BUFF_S0, "\tsudo rm $(includedir)/%s.%c\n", CE.sName, tolower(CE.cLang));
				else if (CE.cLang == 'S')
				  {
					if (strstr(CE.sName, "complete") > 0)	// script names including 'complete' assumed to be bash completion
						snprintf(sBuff, BUFF_S0, "\tsudo rm $(completedir)/%s.sh\n", CE.sName);
					else
						snprintf(sBuff, BUFF_S0, "\tsudo rm $(bindir)/%s.sh\n", CE.sName);
				  }
				else if (CE.iType == CE_PROG_T0 && CE.cMain == CE_MAIN_T0)
					snprintf(sBuff, BUFF_S0, "\tsudo rm $(bindir)/%s\n", CE.sName);

				if (sBuff[0] != 0) ut_check((ios=fputs(sBuff, fpN)), "write %d", ios);
			  }
		  }
		else
			ut_error("unknown marker %.3s", &sBuff[2]);
	  }

	char cMain = CE_MAIN_T0;						// 1st list main executable programs, then functions
	for (i=0; i < iE;)
	  {
		sp=spE+i;
		if (sp->cMain == cMain && ce_gen_make_source(&spE, &iE, &iEmax, sp->sName, sp->iType) > 0)
			ce_gen_make_out(&sp, fpN);				// only build each source file once

		if (++i == iE && cMain == CE_MAIN_T0)		// 1st time through list pick out the main executables
		  {
			cMain=' ';								// 2nd time pick out the project's functions
			i=0;
		  }
	  }

	ios=0;											// all finished ok

error:
	if (spE != NULL) free(spE);
	if (fpT != NULL) fclose(fpT);
	if (fpN != NULL) fclose(fpN);
	cef_main(FA_CLOSE, 0);
	return ios;
  };
