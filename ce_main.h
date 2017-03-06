//--------------------------------------------------------------
//
// details scanned from source and object files to index all programs and their dependencies
//
//	GNU GPLv3 licence	clice (CE) - command line coding Ecosystem by Andrew Bennington 2015 <www.benningtons.net>
//
//--------------------------------------------------------------

#ifndef __CE_MAIN_INCLUDED__
#define __CE_MAIN_INCLUDED__

//------------------------------clice utility function prototypes
int		cef_main(int, char*);		// CE database handler
int		ce_args(int, char**);		// parse clice command-line arguments

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
#define CEF_CODE_B0		0x00001000	// Coding example
#define CEF_LANG_B0		0x00002000	// Language code - 'C', 'F'ortran
#define CEF_SIZE_B0		0x00004000	// Object size
#define CEF_COUNT_B0	0x00008000	// Counter field

					// The following are common combinations of fields - usually read or updated together
#define CEF_LAST_COMP_B0	(CEF_CDATE_B0 +	CEF_CTIME_B0)
#define CEF_LAST_MOD_B0		(CEF_MDATE_B0 +	CEF_MTIME_B0)

//------------------------------Identify the size of any blob/string fields
#define	CE_NAME_S0		30
#define	CE_DIR_S0		30
#define	CE_SOURCE_S0	30
#define	CE_PROJECT_S0	3
#define	CE_DESC_S0		60
#define	CE_CODE_LINE_S0	60

//------------------------------Identifiers for the type of each CE item and link types to other CE items
#define	CE_PROJ_T0	0		// Project master records
#define	CE_PROG_T0	1		// Programs and functions
#define CE_HEAD_T0	2		// Header/library files
#define CE_SYSF_T0	3		// system functions
#define CE_SYSH_T0	4		// system header files

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
	char	sName[CE_NAME_S0];			// Null terminated module name
	char	sDir[CE_DIR_S0];			// Null terminated source directory of module
	char	sSource[CE_SOURCE_S0];		// Null terminated source file of module
	char	sProject[CE_PROJECT_S0];	// Null terminated project code #TODO could get this from end of sDir instead of using db
	char	sDesc[CE_DESC_S0];			// Null terminated module description
	char	sCode[CE_CODE_LINE_S0];		// Null terminated source code example
	char	cLang;			// Language code - 'C', 'F'ortran
							// --- Fields that are not stored in the db but managed by the filehandler
	char	iCount;			// result of any SELECT COUNTS
  } CE, *spCE;


//------------------------------And now all again for the ce_link table
					// These field identifiers are declared in the order of columns in the db table.
					//	(not essential but it makes generated SQL code easier to check)
#define CEF_LINK_ID_B0		0x00000001	// Unique primary key number - allocated by SQL
#define CEF_LINK_NAME_B0	0x00000002	// This CE item/module/record name
#define CEF_LINK_CALLS_B0	0x00000004	// Item/module/record that it is linked to
#define CEF_LINK_NTYPE_B0	0x00000008	// Type of item that Name refers to
#define CEF_LINK_CTYPE_B0	0x00000010	// Type of item that Calls refers to
#define CEF_LINK_CODE_B0	0x00000020	// Source code line where relationship link is made
#define CEF_LINK_TIME_B0	0x00000040	// Time stamp for updating links

struct CEL_FIELDS
  {
	int		bmField;				// bitmap of selected fields/columns. i.e. which fields to read/update
	int		iNo;					// Unique primary key number - allocated by SQL
	int		iTime;					// Time of last update - used as a marker to see what hasn't changed
	char	sName[CE_NAME_S0];		// Null terminated module name
	char	sCalls[CE_NAME_S0];		// Null terminated name of called module
	char	sCode[CE_CODE_LINE_S0];	// Null terminated snippet of source code
	int		iNtype;					// sName's item type (program, header, etc...)
	int		iCtype;					// sCalls' item type
  } CEL, *spCEL;

#endif
