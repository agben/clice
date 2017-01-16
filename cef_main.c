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
//
//	GNU GPLv3 licence	clice by Andy Bennington 2016 [www.benningtons.net]
//
//--------------------------------------------------------------

#include <stdio.h>			// for printf

#include <fa_def.h>			// file/db actions
#include <ce_main.h>		// definition of memory structure holding interim CE data
#include <ce_main_def.h>	// database, table and column definitions for CE
#include <ut_error.h>		// error logging and debug macros

int cef_main(int bmAction, char *cpSQL)
{
	int ios;				// i/o status


	if (bmAction & FA_LINK)								// Record a link between two CE items
	  {
		CE.bmField=0;									// Override any field flags to establish this link
		CEL.bmField=CEF_LINK_ID_B0;						// Just get the ID to confirm if this link already exists
		if (cef_main(FA_READ+FA_STEP,					// Iterative call to this funtion
			"cl.name = % AND cl.ntype = % AND cl.calls = % AND cl.ctype = %") == FA_OK_IV0)	// Are these modules already linked?
		  {
			ios=FA_UPDATE+FA_KEY2;						// Yes so update with current time marker
			CEL.bmField=CEF_LINK_TIME_B0;				// Only need to update the time stamp
		  }
		else
		  {
			printf("CE: now using ");					// No? a new link has been established
			if (CEL.iCtype == CE_PROG_T0)
				printf("function ");
			else if (CEL.iCtype == CE_HEAD_T0)
				printf("header file ");
			else
				printf("unknown type ");

			printf("%s\n", CEL.sCalls);
			ios=FA_WRITE;								// Insert into db
			CEL.bmField=FA_ALL_COLS_B0;
		  }
		ios=cef_main(ios, 0);
	  }
	else if (bmAction & FA_PURGE)						// Remove links that no longer exist in the source
	  {
		CE.bmField=0;									// Override any field flags to find links with old timestamps
		CEL.bmField=CEF_LINK_CALLS_B0;
		ut_check(ios = cef_main(FA_READ,
			"cl.name = % AND cl.ntype = % AND cl.ctype = % AND cl.time <> %") == FA_OK_IV0,
				"purge read");
		ios=0;
		while (cef_main(FA_STEP,0) == FA_OK_IV0)
		  {
			printf("CE: no longer using %s\n", CEL.sCalls);
			ios=1;
		  }

		if (ios == 1)
		  {
			CEL.bmField=FA_ALL_COLS_B0;
			ios=cef_main(FA_DELETE,
				"cl.name = % AND cl.ntype = % AND cl.ctype = % AND cl.time <> %");
		  }
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
