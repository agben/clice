//--------------------------------------------------------------
//
//	Parse command-line arguments passed to clice commands.
//					clice - command line coding ecosystem
//
//	usage:	status = ce_args(argc, argv)
//		where:	argc = count of arguments passed
//				argv = pointer to an array of pointers to arguments
//		returns:	-1 = error or message displayed so abort running command
//					>= 0 number of next argument for calling program to process
//
//	ce_help()		- output basic clice help advice
//	ce_version()	- output version, copyright and licence details for clice
//
//	GNU GPLv3+ licence	clice by Andrew Bennington Feb-2016 <www.benningtons.net>
//
//--------------------------------------------------------------

#include <getopt.h>		// for getopt_long - argument parsing
#include <stdio.h>		// for printf

#include <ce_main.h>	// clice database fields and definitions


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
	printf("              edp <project-name>\n");
	printf("                      to relocate to the project directory and update its clice config file\n");
	printf("      Scan program details and build a documentation database with:\n");
	printf("              ce_scan_bash <bash-script>.sh\n");
	printf("              ce_scan_c <source-file>.c\n");
	printf("              ce_scan_obj <object>.ce\n");
	printf("                      where object = output from objdump\n");
	printf("              ce_scan_project .<project>.clice\n");
	printf("                      to upload project config file changes\n");
	printf("      Manage program documentation with:\n");
	printf("              clice\n\n");
	printf("For general notes on clice: <www.benningtons.net>\n");
  };

void ce_version(void)
  {
	printf("clice - the Command-LIne Coding Ecosystem) v0.1\n");
	printf("        versioning = major.minor.feature-release.bugfix\n\n");
	printf("Copyright (C) Andrew Bennington 2015-2017 <www.benningtons.net>\n");
	printf("Licence GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n");
	printf("This is free software: you are free to change and redistribute it.\n");
	printf("There is NO WARRANTY, to the extent permitted by law\n");
  };

int ce_args(int argc, char **argv)
  {
	static struct option long_options[] = { 		// valid arguments: name, has_arg(yes=1, no=0, opt=2), flag, val
					{"help",	no_argument,		0,	0},			// 0	Keep this order for parsing after getopt_long
					{"version",	no_argument,		0,	0},			// 1
					{"project",	required_argument,	0,	0},			// 2
					{0,			0,					0,	0}
	};
//					{'\0',		0,	'\0',	0}

	int option_index = 0;
	int	i;


	while ((i=getopt_long(	argc,					//number of arguments
							argv,					//argument values - an array of pointers to each argument
							"",						//permitted short arguments  i.e. -v (none permitted)
							long_options,			//permitted long arguments   i.e. --version
							&option_index)) != -1)
	  {
		if (i == '?' || i != 0)				// invalid arg or arg qualifier so abort
			return -1;
		else if (option_index == 0)			// --help
		  {
			ce_help();
			return -1;						// ignore other args
		  }
		else if (option_index == 1)			// --version
		  {
			ce_version();
			return -1;						// ignore other args
		  }
		else if (option_index == 2)			// --project
		  {
			for (i=0; i < CE_PROJECT_S0-1; i++)
				CE.sProject[i]=toupper(optarg[i]);
			CE.sProject[i]='\0';
		  }
	  }

	return optind;
  }
