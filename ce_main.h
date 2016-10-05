//--------------------------------------------------------------
//
// details scanned from source and object files to index all programs and their dependencies
//
//	GNU GPLv3 licence	clice (CE) - command line coding Ecosystem by Andrew Bennington 2015 <www.benningtons.net>
//
//--------------------------------------------------------------

#ifndef __CE_MAIN_INCLUDED__
#define __CE_MAIN_INCLUDED__

#include <fa_sql_def.h>			// common SQL definitions and parameters

//------------------------------Identify the tables in this database - ce_main.db
#define	CE_TABLE_M0		2		// Number of tables in cd_main.db
#define	CE_MAIN_TABLE_P0	0	// The main CE module data table
#define	CE_LINK_TABLE_P0	1	// Table used to show links between CE modules

#define	CE_KEY_M0		5		// Number of keys defined for the SQL generator

//------------------------------Identify the database handler and its field identifiers
int	cef_main(int, char*);		// CE database handler
int	iCEField[CE_TABLE_M0];		// field selection bitmaps. i.e. which fields to read or update
								// used between app and file handler - cef_main
								// 1 bit per field - make a 2d array if more fields added

//------------------------------Other ce utility functions
void	ce_help(void);			// display basic ce help
int	ce_menu(char **, char **);	// menu management for ce_ctrl
void	ce_version(void);		// display version, copyright and licence details

//------------------------------Identify field identifiers in the order of columns in the db table.
								//	(order not essential but it makes generated SQL code easier to check)
#define	CEF_ID_B0		0x00000001	// Unique primary key number - allocated by SQL
#define CEF_NAME_B0		0x00000002	// CE item/module/record name
#define CEF_TYPE_B0		0x00000004	// Type of module - prog, library, parameter, etc
#define CEF_STATUS_B0	0x00000008	// Status of module
#define CEF_DIR_B0		0x00000010	// Source directory of module
#define CEF_CDATE_B0	0x00000020	// Last compiled date
#define CEF_CTIME_B0	0x00000040	// Last compiled time
#define CEF_MDATE_B0	0x00000080	// Last modified date
#define CEF_MTIME_B0	0x00000100	// Last modified time
#define CEF_DESC_B0		0x00000200	// Module description
#define CEF_LANG_B0		0x00000400	// Language code - 'C', 'F'ortran
#define CEF_SIZE_B0		0x00000800	// Object size

					// The following are common combinations of fields - usually read or updated together
#define CEF_LAST_COMP_B0	(CEF_CDATE_B0 +	CEF_CTIME_B0)
#define CEF_LAST_MOD_B0		(CEF_MDATE_B0 +	CEF_MTIME_B0)

//------------------------------Identify the size of any blob/string fields
#define	CE_NAME_S0	30
#define	CE_DIR_S0	30
#define	CE_DESC_S0	40

#define	CE_FIELD_M0	12		// total number of int + blob fields

//------------------------------Identifiers for the type of each CE item and link types to other CE items
#define CE_PRG_T0	1		//	Program
#define CE_SYS_T0	2		//	System routine call
#define CE_LIB_T0	3		//	Library entry
//#define CE_PAR_T0	4		//	Parameter
//#define CE_PRF_T0	5		//	Proforma
//#define CE_CLF_T0	7		//	Called from
//#define CE_COM_T0	8		//	Common area
//#define CE_AMD_T0	9		//	Ammendment number

//------------------------------Declare some meaningful field names to map to the unpacked data
struct CE_FIELDS
  {
	int		iNo;			// Unique primary key number - allocated by SQL
	int		iType;			// Type of module - prog, library, parameter, etc
	int		iStatus;		// Status of module
	int		iCDate;			// Last compiled date
	int		iCTime;			// Last compiled time
	int		iMDate;			// Last modified date
	int		iMTime;			// Last modified time
	int		iSize;			// Object size
	char	cName[CE_NAME_S0];	// Null terminated module name
	char	cDir[CE_DIR_S0];	// Null terminated source directory of module
	char	cDesc[CE_DESC_S0];	// Null terminated module description
	char	cLang;			// Language code - 'C', 'F'ortran
  } CE, *spCE;


//------------------------------And now all again for the ce_link table
					// These field identifiers are declared in the order of columns in the db table.
					//	(not essential but it makes generated SQL code easier to check)
#define CEF_LINK_ID_B0		0x00000001	// Unique primary key number - allocated by SQL
#define CEF_LINK_NAME_B0	0x00000002	// This CE item/module/record name
#define CEF_LINK_CALLS_B0	0x00000004	// Item/module/record that it is linked to
#define CEF_LINK_REL_B0		0x00000008	// Relationship of that link- i.e. call to program or inclusion of library
#define CEF_LINK_TIME_B0	0x00000010	// Time stamp for updating links

struct CEL_FIELDS
  {
	int		iNo;			// Unique primary key number - allocated by SQL
	int		iTime;			// Time of last update - used as a marker to see what hasn't changed
	char	cName[CE_NAME_S0];	// Null terminated module name
	char	cCalls[CE_NAME_S0];	// Null terminated name of called module
	char	cRel;			// Type of relationship to cCalls
  } CEL, *spCEL;

#endif
