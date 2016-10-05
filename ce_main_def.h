//--------------------------------------------------------------
//
// Database definitions for clice, the command line coding ecosystem (CE)
//
// Intended for cef_main the file/database handler and ce_main.sql which generates the database.
//	By having both use these same definitions then discrepencies should be avoided.
//
//	GNU GPLv3 licence	clice by Andrew Bennington 2016 [www.benningtons.net]
//
//#TODO this file should be used to generate ce_main.sql
//
//--------------------------------------------------------------

#ifndef __CE_MAIN_DEF_INCLUDED__
#define __CE_MAIN_DEF_INCLUDED__

#include <ce_main.h>		// definition of coding ecosystem (CE) data structures
#include <fa_sql_def.h>		// Generic SQL table and column definitions


struct sql_column CEF[CE_TABLE_M0][CE_FIELD_M0] =	//Declare the columns used in each table
  {
	{//name		column flags	where to extract SQL data to	max size of column data
	{"id",	(	SQL_COL_INT_B0+
				SQL_COL_PRIME_B0+
				SQL_COL_AUTO_B0),	(char*)&CE.iNo,				SQL_FIELD_INT_S0},
	{"name",	SQL_COL_BLOB_B0,	(char*)&CE.cName,			CE_NAME_S0},
	{"type",	SQL_COL_INT_B0,		(char*)&CE.iType,			SQL_FIELD_INT_S0},
	{"status",	SQL_COL_INT_B0,		(char*)&CE.iStatus,			SQL_FIELD_INT_S0},
	{"dir",		SQL_COL_BLOB_B0,	(char*)&CE.cDir,			CE_DIR_S0},
	{"cdate",	SQL_COL_INT_B0,		(char*)&CE.iCDate,			SQL_FIELD_INT_S0},
	{"ctime",	SQL_COL_INT_B0,		(char*)&CE.iCTime,			SQL_FIELD_INT_S0},
	{"mdate",	SQL_COL_INT_B0,		(char*)&CE.iMDate,			SQL_FIELD_INT_S0},
	{"mtime",	SQL_COL_INT_B0,		(char*)&CE.iMTime,			SQL_FIELD_INT_S0},
	{"desc",	SQL_COL_BLOB_B0,	(char*)&CE.cDesc,			CE_DESC_S0},
	{"lang",	SQL_COL_CHAR_B0,	(char*)&CE.cLang,			SQL_FIELD_CHAR_S0},
	{"size",	SQL_COL_INT_B0,		(char*)&CE.iSize,			SQL_FIELD_INT_S0}
	},
	{//name		column flags	where to extract SQL data to	max size of column data
	{"id",	(	SQL_COL_INT_B0+
				SQL_COL_PRIME_B0+
				SQL_COL_AUTO_B0),	(char*)&CEL.iNo,			SQL_FIELD_INT_S0},
	{"name",	SQL_COL_BLOB_B0,	(char*)&CEL.cName,			CE_NAME_S0},
	{"calls",	SQL_COL_BLOB_B0,	(char*)&CEL.cCalls,			CE_NAME_S0},
	{"rel",		SQL_COL_CHAR_B0,	(char*)&CEL.cRel,			SQL_FIELD_CHAR_S0},
	{"time",	SQL_COL_INT_B0,		(char*)&CEL.iTime,			SQL_FIELD_INT_S0},
	}
  };

struct sql_table CET[CE_TABLE_M0] =		//Declare the tables used in ce_main.db
  {// name		alias	Column count	pointer to column details
	{"ce_main",	"ce",	CE_FIELD_M0,	&CEF[0][0],},
	{"ce_link",	"cl",	5,				&CEF[1][0]}
  };

struct sql_db CEB =				//Declare database details for clice_main.db
  {	// db name		  Tables	max columns		max keys	table defs
	"clice_main.db",	2,		CE_FIELD_M0,	CE_KEY_M0,	&CET[0],
//	key scripts
		{"ce.id = %",				// 0
		 "ce.name = %",				// 1
		 "cl.id = %",				// 2
		 "cl.name = % AND cl.time <> %",	// 3
		 "cl.name = % AND cl.calls = %",	// 4
		 "ce.name LIKE % AND ce.type = % ORDER BY ce.name ASC"}	// 5
  };

#endif
