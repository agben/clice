//--------------------------------------------------------------
//
// Utility functions for CE - Coding Ecosystem
//
//	ce_help()		- output basic CE help advice
//	ce_version()	- output version, copyright and licence details for CE
//
//	GNU GPLv3+ licence	CE by Andrew Bennington 2016 <www.benningtons.net>
//
//--------------------------------------------------------------

#include <stdio.h>	//for printf

void ce_help(void)
  {
	printf("CE - Coding Ecosystem\n");
	printf("USAGE:\n");
	printf("      Scan program details with:\n");
	printf("              ce_scan <source> --language <lang>\n");
	printf("                      where spurce = program source name and lang = C or F'ortran'\n");
	printf("              ce_scan_obj <object>\n");
	printf("                      where object = compiled object code\n");
	printf("      Manage program documentation with:\n");
	printf("              ce_ctrl\n\n");
	printf("For general notes on CE: <www.benningtons.net>\n");
  };

void ce_version(void)
  {
	printf("ce_scan (Coding Ecosystem) v0.1.1\n");
	printf("Copyright (C) Andrew Bennington 2015 <www.benningtons.net>\n");
	printf("Licence GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n");
	printf("This is free software: you are free to change and redistribute it.\n");
	printf("There is NO WARRANTY, to the extent permitted by law\n");
  };
