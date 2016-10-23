//--------------------------------------------------------------
//
//	File/Database handler for ce_main.db - the clice database of source code details
//
//	usage:	status = cef_main (action, SQL)
//		where action is a bitmap of filehandler commands - see fa_def.h
//		and SQL is an optional SQL script to pass onto certain actions
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
//
//	GNU GPLv3 licence	clice by Andy Bennington 2016 [www.benningtons.net]
//
//--------------------------------------------------------------

#include <fa_def.h>			// file/db actions
#include <ce_main.h>		// definition of memory structure holding interim CE data
#include <ce_main_def.h>	// database, table and column definitions for CE

int cef_main(int bmAction, char *cpSQL)
{
	CET[CE_MAIN_TABLE_P0].bmField = CE.bmField;			// In this example database columns match the CE and DEL structures used in clice
	CET[CE_LINK_TABLE_P0].bmField = CEL.bmField;		// so no conversions are required.

    return fa_handler(bmAction, &CEB, cpSQL);
}
