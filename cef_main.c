//--------------------------------------------------------------
//
//	File/Database handler for ce_main.db - the clice database of source code details
//
//	usage:	status = cef_main (action, SQL)
//		where action is a bitmap of filehandler commands - see fa_def.h
//		and SQL is an optional SQL script to pass onto certain actions
//
//	Returns:	The I/O success/error status.
//
//	Also dependent on data defintions in ce_main.h and ce_main_def.h such as:
//		CEB.iLun to hold the database's position in the fa_lun.h structure of database and transaction handles.
//		CET.bField a bitmap used to identify which fields/columns to read or update.
//
//
//	Whilst this local application filehandler is mainly just a wrapper for the generic fa_handler() it does provide
//		an opportunity to insert triggers relevant to the application (i.e. audit updates) and for database data to under-go
//		any necessary conversions before use by the calling application (i.e. a date column in a database table could be
//		converted into different formats ready for use by the calling app)
//		Local acions for this function include:
//			FA_LINK	- Update or create a CEL link row to show the relationship between two CE items.
//						The CEL columns need to be setup ready to add or update a row.
//			FA_PURGE- Remove CEL link rows that don't match the CEL timestamp
//			FA_ADD	- Adding a new CE main row so check if it exists and then write or update, with suitable user message
//
//	GNU GPLv3 licence	clice by Andy Bennington 2016 [www.benningtons.net]
//
//--------------------------------------------------------------

#include <stdio.h>			// for printf
#include <string.h>			// for memcpy

#include <fa_def.h>			// file/db actions
#include <ce_main.h>		// definition of memory structure holding interim CE data
#include <ce_main_def.h>	// database, table and column definitions for CE
#include <ut_error.h>		// error logging and debug macros

#define	CE_MODULE_LINK_M0	40	// max links from one module to others

int cef_main(int bmAction, char *cpSQL)
{
	int i;
	int ios;				// i/o status

	int iModule=0;									// count of modules
	char sModule[CE_MODULE_LINK_M0][CE_NAME_S0];	// list of module names
	int	iType[CE_MODULE_LINK_M0];					// list of module types


	if (bmAction & FA_COUNT)							// Read a count of matching rows
	  {
		CET[CE_MAIN_TABLE_P0].bmField = CE.bmField;		// In this example database columns match the CE and CEL structures used in
		CET[CE_LINK_TABLE_P0].bmField = CEL.bmField;	//	clice so no conversions are required.
//		CET[CE_MAIN_TABLE_P0].bmField = CEF_COUNT_B0;	// All counts go to the same meta field
//		CET[CE_LINK_TABLE_P0].bmField = 0;
		i=FA_COUNT+FA_READ+FA_STEP;
		ios=fa_handler(i, &CEB, cpSQL);
	  }
	else if (bmAction & FA_LINK)						// Record a link between two CE items
	  {
		CE.bmField=0;									// Override any field flags to establish this link
		CEL.bmField=CEF_LINK_ID_B0;						// Just get the ID to confirm if this link already exists
		ios=0;
		for (i=0; i < 2 && ios == 0; i++)				// Is there an existing link with this item?
		  {
			if (i == 1) CEL.iCtype+=2;					// not your function/header so is it a known system one?
			if (cef_main(FA_READ+FA_STEP,				// Iterative call to this function
				"cl.name = % AND cl.ntype = % AND cl.calls = % AND cl.ctype = %") == FA_OK_IV0)	// Are these modules already linked?
			  {
				ios=FA_UPDATE+FA_KEY2;					// Yes so update with current time marker
				CEL.bmField=CEF_LINK_TIME_B0;			// Only need to update the time stamp
			  }
		  }

		if (ios == 0)									// no existing link to this local or system item found
		  {
			printf("CE: now using ");					// so a new link has been established

			memcpy(CE.sName, CEL.sCalls, CE_NAME_S0);
			CE.iType=CEL.iCtype-2;						// first try finding the funcion/header as a locally created item

			CE.bmField=CEF_ID_B0;						// Check if clice knows about this module?
			CEL.bmField=0;
			if (cef_main(FA_READ+FA_STEP,
					"ce.name = % AND ce.type = %") != FA_OK_IV0)
			  {
				CE.iType+=2;							// else try finding it as a system funcion/header
				if (cef_main(FA_READ+FA_STEP,
					"ce.name = % AND ce.type = %") != FA_OK_IV0)
				  {
					printf("*NEW* ");					// unknown on clice so flag as new

					CE.iCDate=CE.iMDate=0;				// initialise (currently) unknown fields
					CE.iCTime=CE.iMTime=0;
					CE.sDir[0]=CE.sCode[0]=CE.sProject[0]=
							CE.sDesc[0]=CE.sSource[0]='\0';
					CE.cLang='?';
					CE.iStatus=0;
					CE.iSize=0;

					CE.bmField=FA_ALL_COLS_B0;			// Going to insert all new fields
					ios=cef_main(FA_WRITE, 0);			// add CE module to clice db
				  }
			  }

			CEL.iCtype=CE.iType;						// record as (system) header/function
			if (CEL.iCtype == CE_PROG_T0)
				printf("function: ");
			else if (CEL.iCtype == CE_HEAD_T0)
				printf("header: ");
			else if (CEL.iCtype == CE_SYSF_T0)
				printf("system function: ");
			else if (CEL.iCtype == CE_SYSH_T0)
				printf("system header: ");
			else
				printf("unknown type: ");
			printf("%s\n", CEL.sCalls);

			ios=FA_WRITE;								// Insert link into db
			CE.bmField=0;
			CEL.bmField=FA_ALL_COLS_B0;
		  }
		ios=cef_main(ios, 0);
	  }
	else if (bmAction & FA_PURGE)						// Remove links that no longer exist in the source
	  {
		for (i=0; i < 2; i++)							// check for local and then again for system headers/functions
		  {
			if (i == 1) CEL.iCtype+=2;					// switch to check for system functions/headers
			CE.bmField=0;								// Override any field flags to find links with old timestamps
			CEL.bmField=CEF_LINK_CALLS_B0;
			ut_check(ios = cef_main(FA_READ,
				"cl.name = % AND cl.ntype = % AND cl.ctype = % AND cl.time <> %") == FA_OK_IV0,
					"purge read");
			ios=0;
			while (cef_main(FA_STEP,0) == FA_OK_IV0)
			  {
				printf("CE: no longer using ");
				if (CEL.iCtype == CE_PROG_T0)
					printf("function: ");
				else if (CEL.iCtype == CE_HEAD_T0)
					printf("header: ");
				else if (CEL.iCtype == CE_SYSF_T0)
					printf("system function: ");
				else if (CEL.iCtype == CE_SYSH_T0)
					printf("system header: ");
				else
					printf("unknown type: ");
				printf("%s\n", CEL.sCalls);

				if (iModule < CE_MODULE_LINK_M0-1)
				  {
					memcpy(sModule[iModule], CEL.sCalls, CE_NAME_S0);
					iType[iModule++]=CEL.iCtype;
				  }
				else
					printf("CE: too many links removed in one edit!\n");

				ios=1;
			  }

			if (ios == 1)
			  {
				CEL.bmField=FA_ALL_COLS_B0;
				ios=cef_main(FA_DELETE,
					"cl.name = % AND cl.ntype = % AND cl.ctype = % AND cl.time <> %");

				for (i=0; i < iModule; i++)			// check if any previously linked modules are nolonger referenced by anything
				  {
					memcpy(CEL.sCalls, sModule[i], CE_NAME_S0);
					CEL.iCtype=iType[i];

					CE.bmField=0;
					CEL.bmField=CEF_ID_B0;
					if (cef_main(FA_READ+FA_STEP,
							"cl.calls = % AND cl.ctype = %") != FA_OK_IV0)
					  {								// nothing else is linked to this item
						if (CEL.iCtype < CE_SYSF_T0)		// if a locally created function or header then you may wish to keep?
							printf("    it is no longer referenced so use clice if you wish to delete it.\n");
						else								// for system items we can decide to auto delete
						  {
							printf("    it is no longer referenced and so removed from clice.\n");
							memcpy(CE.sName, sModule[i], CE_NAME_S0);
							CE.iType=iType[i];

							CE.bmField=FA_ALL_COLS_B0;
							CEL.bmField=0;
							cef_main(FA_DELETE, "ce.name = % AND ce.type = %");		// status not checked as may not exist
						  }
					  }
				  }
			  }
		  }
	  }
	else if (bmAction & FA_ADD)						// Adding or updating a new clice module
	  {
		CE.bmField=CEF_ID_B0;						// only need to read ID to confirm if this module exists in CE
		CEL.bmField=0;
		if (cef_main(FA_READ+FA_STEP,
			"ce.name = % AND ce.type = %") == FA_NODATA_IV0)	// module not found so create a new CE module
		  {
			if (CE.iType == CE_HEAD_T0)
				printf("CE: New header file added - %s\n", CE.sName);
			else
				printf("CE: New program file added - %s\n", CE.sName);

			CE.iType+=2;							// check if this module has been setup as a system function/header?
			if (cef_main(FA_READ+FA_STEP,
				"ce.name = % AND ce.type = %") == FA_OK_IV0)	// it has, so needs converting to a local function/header
			  {
				memcpy(CEL.sCalls, CE.sName, CE_NAME_S0);
				CEL.iCtype=CE.iType-2;

				CE.bmField=0;
				CEL.bmField=CEF_LINK_CTYPE_B0;					// just update the called type
				sprintf(sModule[0], "cl.calls = %% AND cl.ctype = %d",
						CE.iType);								// search for system type and convert to local type
				cef_main(FA_UPDATE, sModule[0]);

				ios=FA_UPDATE;
				CE.bmField=FA_ALL_COLS_B0-CEF_ID_B0-
							CEF_NAME_B0;
				CEL.bmField=0;
			  }
			else
			  {
				CE.bmField=FA_ALL_COLS_B0;			// Going to insert all new fields
				ios=FA_WRITE;						// Write a new CE module to db
			  }
			CE.iType-=2;							// revert back to original local function/header type
		  }
		else										// exists so update last modified date/time
		  {
			CE.bmField=CEF_LAST_MOD_B0+CEF_DESC_B0;	// and description (in case changed)
			ios=FA_UPDATE;							// update CE module in db
		  }
		ios=cef_main(ios, 0);						// add/update CE module in db
	  }
	else
	  {
		CET[CE_MAIN_TABLE_P0].bmField = CE.bmField;		// In this example database columns match the CE and CEL structures used in
		CET[CE_LINK_TABLE_P0].bmField = CEL.bmField;	//	clice so no conversions are required.
		ios=fa_handler(bmAction, &CEB, cpSQL);
	  }

error:
    return ios;
}
