//--------------------------------------------------------------
//
// Parse input for program module names
//
//	usage:	ce_parse_name <source, destination>
//		where	source	= start of a module name within original source or analysis file
//			destination = where to extract the module name to
//
//	GNU GPLv3+ licence	clice - The command-line coding ecosystem by Andrew Bennington 2020 <www.benningtons.net>
//
//--------------------------------------------------------------

#include <ce_main.h>	// CE database
#include <stdio.h>		// for printf


void ce_parse_name(char *cpSrc, char *cpDest)		// Extract function/header names from source file
  {
	int i = 1;
	char *cpDest2 = cpDest;

	for (;	i++ < CE_NAME_S0 &&						// extract name until invalid character or name is full
			*cpSrc != '>' &&
			*cpSrc != '+' &&
			*cpSrc != '-' &&
			*cpSrc != '(' &&
			*cpSrc != '\"' &&
			*cpSrc != ' ' &&
			*cpSrc != '\0' &&
			*cpSrc != '\n' &&
			*cpSrc != '.';		 cpSrc++)
	  *cpDest++ = *cpSrc;

	*cpDest='\0';									// null terminate name
	if (i >= CE_NAME_S0) printf ("Warning: module name truncated to:%s\n", cpDest2);
  };
