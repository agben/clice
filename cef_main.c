//--------------------------------------------------------------
//
// File/Database handler for ce_main.db - clice (CE) the command line coding ecosystem
//
//	usage:	status = cef_main (action, SQL)
//		where action is a bitmap of filehandler commands - see fa_def.h
//		and SQL is an optional SQL script to pass onto certain actions
//
//		FA_OPEN		- Open Database
//		FA_CLOSE	- Close Database
//		FA_READ		- Prepare a SELECT command. Can be used with FA_STEP to return the result of the 1st STEP
//		FA_WRITE	- Prepare an INSERT command to add a row to the database
//		FA_UPDATE	- Prepare an UPDATE command to update selected fields in the database
//		FA_PREPARE	- An adhoc query so pass the SQL instruction on to the sql_handler
//		FA_STEP		- Return the next row of data from an FA_READ
//		FA_FINALISE	- Tidily close a SELECT-STEP-FINALISE loop - other commands will also trigger this
//		FA_EXEC		- Pass on a passed SQL instruction for execution in a single SELECT-STEP-FINALISE action
//		FA_DELETE	- Prepare a DELETE command to remove a row from the database
//
//	Also dependent on data defintions in ce_main.h and ce_main_def.h
//
//	GNU GPLv3 licence	clice by Andrew Bennington 2015 [www.benningtons.net]
//
//--------------------------------------------------------------

#include <fa_def.h>			// file/db actions
#include <ce_main.h>		// definition of memory structure holding interim CE data
#include <ce_main_def.h>	// database, table and column definitions for CE
#include <ut_error.h>		// error handling and debug functions

int cef_main(int iAction, char *cpSQL)
{
	char cBuff[SQL_BUFFER_S0];		// SQL command input buffer	#TODO - use malloc and a common SQL size
	int i, j;
	int ios = 0;
	char *cp = &cBuff[0];


	ut_debug("action:%x", iAction);

	if (iAction & (FA_PREPARE+FA_EXEC))					// Use the passed SQL script for adhoc actions
		cp=cpSQL;
	else if (iAction & (FA_WRITE+FA_READ+FA_UPDATE+FA_DELETE))	// generate an SQL script from the details passed
	 {
		ut_check(fa_sql_generator(	iAction,			// Pass on the action
									&iCEField[0],		// Pointer to requested fields bitmap
									&CEB,				// Database definition
									cpSQL,				// pass any SQL script fed into the filehandler
									cp) == 0,			// pointer to output buffer for generated scipt
				"SQL gen fail");						// will jump to error: if a problem
		cp=&cBuff[0];									// point back to the start ready for passing
		if (!(iAction & FA_READ)) iAction=FA_EXEC;		// SQL script is prepared so now execute it
	 }

	if (iAction & (FA_PREPARE+FA_FINALISE+FA_EXEC+FA_READ+FA_OPEN+FA_CLOSE))	// Pass these SQL commands straight through
	 {
		if (iAction & FA_READ)							// FA_READ will cause a PREPARE followed by a STEP
			i=FA_PREPARE;
		else
			i=iAction;

		if (i & (FA_PREPARE+FA_EXEC)) ut_debug("SQL=%s", cp);	// check on prepared SQL scripts

		ios=fa_sql_handler(	i,							// Pass on the action
							CE_MAIN_L0,					// Database handle
							cp,							// SQL command
							&CEB);						// Database definition
		ut_check (ios == 0,"%d", ios);					// jumps to error: if not true
	 }

	else
		ut_check(!(ios & FA_STEP), "unknown: %d", iAction);		// Ignore FA_STEP as dealt with below


	if (iAction & FA_STEP)								// Step through rows from a previously prepared SELECT
	 {
		ios=fa_sql_handler(	FA_STEP,					// Action
							CE_MAIN_L0,					// Database handle
							0,							// not used
							&CEB);						// Field definitions
		if (ios == FA_OK_IV0)							// Data returned
		 {												// #TODO remove following debug before publishing
			for (i=0; i < CEB.iTab; i++)
			 {
				for (j=0; j < CET[i].iCol; j++)
				 {
					if ((iCEField[i]>>j) & 1)
					 {
						if (CEF[i][j].iFlag & SQL_COL_INT_B0)
						 {
							ut_debug(	"unpack:%s :%d",
										CEF[i][j].cName,
										*(int *)CEF[i][j].cPos);
						 }
						else
						 {
							ut_debug(	"unpack:%s :%s",
										CEF[i][j].cName,
										CEF[i][j].cPos);
						 }
					 }
				 }
			 }
		 }
		else
			ut_check(	ios == FA_NODATA_IV0,
						"step error %d", ios);			// Ignore no data found or end of row messages
	 }

error:
	return ios;
}
