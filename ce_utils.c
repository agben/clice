//--------------------------------------------------------------
//
// Utility functions for clice - the command-line coding ecosystem
//
//	ce_help()		- output basic clice help advice
//	ce_version()	- output version, copyright and licence details for clice
//
//	GNU GPLv3+ licence	clice by Andrew Bennington 2016 <www.benningtons.net>
//
//--------------------------------------------------------------

#include <stdio.h>	//for printf

void ce_help(void)
  {
	printf("clice - the Command-LIne Coding Ecosystem\n");
	printf("USAGE:\n");
	printf("      Traverse project directories and update source files with:\n");
	printf("              edc <project-name>\n");
	printf("                      to relocate to the project directory\n");
	printf("              edc <source-file>\n");
	printf("                      to relocate to the project directory and select source-file\n");
	printf("              edm <project-name>\n");
	printf("                      to relocate to the project directory and update its makefile\n");
	printf("      Scan program details and build a documentation database with:\n");
	printf("              ce_scan <source-file> --language <lang>\n");
	printf("                      where source-file = program source name and lang = C or F'ortran'\n");
	printf("              ce_scan_obj <object>\n");
	printf("                      where object = compiled object code\n");
	printf("      Manage program documentation with:\n");
	printf("              clice\n\n");
	printf("For general notes on clice: <www.benningtons.net>\n");
  };

void ce_version(void)
  {
	printf("clice - the Command-LIne Coding Ecosystem) v0.1.3.0\n");
	printf("        versioning = major.minor.bugfix.documentation\n\n");
	printf("Copyright (C) Andrew Bennington 2015 <www.benningtons.net>\n");
	printf("Licence GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n");
	printf("This is free software: you are free to change and redistribute it.\n");
	printf("There is NO WARRANTY, to the extent permitted by law\n");
  };
