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

//------------------------------clice utility functions
int		cef_main(int, char*);		// CE database handler
void	ce_help(void);				// display basic ce help
int		ce_menu(char **, char **);	// menu management for ce_ctrl
void	ce_version(void);			// display version, copyright and licence details

//------------------------------Identify field identifiers in the order of columns in the db table.
					//	(order not essential but it makes generated SQL code easier to check)
#define	CEF_ID_B0		0x00000001	// Unique primary key number - allocated by SQL
#define CEF_NAME_B0		0x00000002	// CE item/module/record name
#define CEF_TYPE_B0		0x00000004	// Type of module - prog, library, parameter, etc
#define CEF_STATUS_B0	0x00000008	// Status of module
#define CEF_DIR_B0		0x00000010	// Source directory of module
#define CEF_SOURCE_B0	0x00000020	// Source file of module
#define CEF_PROJECT_B0	0x00000040	// Project code name
#define CEF_CDATE_B0	0x00000080	// Last compiled date
#define CEF_CTIME_B0	0x00000100	// Last compiled time
#define CEF_MDATE_B0	0x00000200	// Last modified date
#define CEF_MTIME_B0	0x00000400	// Last modified time
#define CEF_DESC_B0		0x00000800	// Module description
#define CEF_LANG_B0		0x00001000	// Language code - 'C', 'F'ortran
#define CEF_SIZE_B0		0x00002000	// Object size

					// The following are common combinations of fields - usually read or updated together
#define CEF_LAST_COMP_B0	(CEF_CDATE_B0 +	CEF_CTIME_B0)
#define CEF_LAST_MOD_B0		(CEF_MDATE_B0 +	CEF_MTIME_B0)

//------------------------------Identify the size of any blob/string fields
#define	CE_NAME_S0		30
#define	CE_DIR_S0		30
#define	CE_SOURCE_S0	30
#define	CE_PROJECT_S0	3
#define	CE_DESC_S0		40
#define	CE_CODE_LINE_S0	60

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
	int		bmField;		// bitmap of selected fields/columns. i.e. which fields to read/update
	int		iNo;			// Unique primary key number - allocated by SQL
	int		iType;			// Type of module - prog, library, parameter, etc
	int		iStatus;		// Status of module
	int		iCDate;			// Last compiled date
	int		iCTime;			// Last compiled time
	int		iMDate;			// Last modified date
	int		iMTime;			// Last modified time
	int		iSize;			// Object size
	char	sName[CE_NAME_S0];	// Null terminated module name
	char	sDir[CE_DIR_S0];	// Null terminated source directory of module
	char	sSource[CE_SOURCE_S0];	// Null terminated source file of module
	char	sProject[CE_PROJECT_S0];	// Null terminated project code #TODO could get this from end of sDir instead of using db
	char	sDesc[CE_DESC_S0];	// Null terminated module description
	char	cLang;			// Language code - 'C', 'F'ortran
  } CE, *spCE;


//------------------------------And now all again for the ce_link table
					// These field identifiers are declared in the order of columns in the db table.
					//	(not essential but it makes generated SQL code easier to check)
#define CEF_LINK_ID_B0		0x00000001	// Unique primary key number - allocated by SQL
#define CEF_LINK_NAME_B0	0x00000002	// This CE item/module/record name
#define CEF_LINK_CALLS_B0	0x00000004	// Item/module/record that it is linked to
#define CEF_LINK_REL_B0		0x00000008	// Relationship of that link- i.e. call to program or inclusion of library
#define CEF_LINK_CODE_B0	0x00000010	// Source code line where relationship link is made
#define CEF_LINK_TIME_B0	0x00000020	// Time stamp for updating links

struct CEL_FIELDS
  {
	int		bmField;				// bitmap of selected fields/columns. i.e. which fields to read/update
	int		iNo;					// Unique primary key number - allocated by SQL
	int		iTime;					// Time of last update - used as a marker to see what hasn't changed
	char	sName[CE_NAME_S0];		// Null terminated module name
	char	sCalls[CE_NAME_S0];		// Null terminated name of called module
	char	sCode[CE_CODE_LINE_S0];	// Null terminated snippet of source code
	char	cRel;					// Type of relationship to cCalls
  } CEL, *spCEL;

//------------------------------Object symbols identified by the nm routine
#define CEL_REL_UNDAT_V0	'C'	// Uninitialised data
#define CEL_REL_UNDEF_V0	'U'	// Undefined symbol - usually indicating a local or system routine
#define CEL_REL_GDATA_V0	'D'	// Initialised data - global
#define CEL_REL_LDATA_V0	'd'	// Initialised data - local
#define CEL_REL_GTEXT_V0	'T'	// Text symbol - global
#define CEL_REL_UNDEF_V0	'U'	// Undefined symbol - usually indicating a local or system routine

#endif
