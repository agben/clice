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

//------------------------------Identify the tables in this database - ce_main.db
#define	CE_TABLE_M0			2	// Number of tables in cd_main.db
#define	CE_MAIN_TABLE_P0	0	// The main CE module data table
#define	CE_LINK_TABLE_P0	1	// Table used to show links between CE modules

#define	CE_KEY_M0		5		// Number of keys defined for the SQL generator

#define	CE_FIELD_M0		14		// max number of int + blob fields


struct fa_sql_column CEF[CE_TABLE_M0][CE_FIELD_M0] =	//Declare the columns used in each table
  {
	{//name		column flags	where to extract SQL data to	max size of column data
	{"id",	(	FA_COL_INT_B0+
				FA_COL_PRIME_B0+
				FA_COL_AUTO_B0),	(char*)&CE.iNo,			FA_FIELD_INT_S0},
	{"name",	FA_COL_BLOB_B0,		(char*)&CE.sName,		CE_NAME_S0},
	{"type",	FA_COL_INT_B0,		(char*)&CE.iType,		FA_FIELD_INT_S0},
	{"status",	FA_COL_INT_B0,		(char*)&CE.iStatus,		FA_FIELD_INT_S0},
	{"dir",		FA_COL_BLOB_B0,		(char*)&CE.sDir,		CE_DIR_S0},
	{"source",	FA_COL_BLOB_B0,		(char*)&CE.sSource,		CE_SOURCE_S0},
	{"project",	FA_COL_BLOB_B0,		(char*)&CE.sProject,	CE_PROJECT_S0},
	{"cdate",	FA_COL_INT_B0,		(char*)&CE.iCDate,		FA_FIELD_INT_S0},
	{"ctime",	FA_COL_INT_B0,		(char*)&CE.iCTime,		FA_FIELD_INT_S0},
	{"mdate",	FA_COL_INT_B0,		(char*)&CE.iMDate,		FA_FIELD_INT_S0},
	{"mtime",	FA_COL_INT_B0,		(char*)&CE.iMTime,		FA_FIELD_INT_S0},
	{"desc",	FA_COL_BLOB_B0,		(char*)&CE.sDesc,		CE_DESC_S0},
	{"lang",	FA_COL_CHAR_B0,		(char*)&CE.cLang,		FA_FIELD_CHAR_S0},
	{"size",	FA_COL_INT_B0,		(char*)&CE.iSize,		FA_FIELD_INT_S0}
	},
	{//name		column flags	where to extract SQL data to	max size of column data
	{"id",	(	FA_COL_INT_B0+
				FA_COL_PRIME_B0+
				FA_COL_AUTO_B0),	(char*)&CEL.iNo,		FA_FIELD_INT_S0},
	{"name",	FA_COL_BLOB_B0,		(char*)&CEL.sName,		CE_NAME_S0},
	{"calls",	FA_COL_BLOB_B0,		(char*)&CEL.sCalls,		CE_NAME_S0},
	{"rel",		FA_COL_CHAR_B0,		(char*)&CEL.cRel,		FA_FIELD_CHAR_S0},
	{"code",	FA_COL_BLOB_B0,		(char*)&CEL.sCode,		CE_CODE_LINE_S0},
	{"time",	FA_COL_INT_B0,		(char*)&CEL.iTime,		FA_FIELD_INT_S0},
	}
  };

struct fa_sql_table CET[CE_TABLE_M0] =		//Declare the tables used in ce_main.db
  {// name		alias	Column count	Fields	pointer to column details
	{"ce_main",	"ce",	CE_FIELD_M0,	0,		&CEF[0][0],},
	{"ce_link",	"cl",	6,				0,		&CEF[1][0]}
  };

struct fa_sql_db CEB =				//Declare database details for clice_main.db
  {	// db path		db name			  Tables	max columns		max keys	LUN	table defs
	"/var/local/",	"clice_main.db",	2,		CE_FIELD_M0,	CE_KEY_M0,	0,	&CET[0],
//	key scripts
		{"ce.id = %",				// 0
		 "ce.name = %",				// 1
		 "cl.id = %",				// 2
		 "cl.name = % AND cl.time <> %",	// 3
		 "cl.name = % AND cl.rel = % ORDER BY cl.calls ASC",	// 4	select all links of type 'cl.rel' from module 'cl.name'
		 "ce.name LIKE % AND ce.type = % ORDER BY ce.name ASC",	// 5	select all matching modules of specified type (program. library, etc...)
		 "cl.calls = % AND cl.rel = % ORDER BY cl.name ASC"}	// 6	select all links of type 'cl.rel' to module 'cl.calls'
  };
#endif
