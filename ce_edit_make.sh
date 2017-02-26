#!/bin/bash
#
# makefile editor for clice (CE) - the command line coding ecosystem
#
# Setup:	alias to a command name in your .bashrc
#			i.e. edm '. ce_edit_make.sh'
#			use '. ' to run in the current shell to benefit from directory and flag changes
# Usage:	edm ce
#			this will change your directory to CE (project name) and open the makefile
#
# Parameter 1 is the project name
#

#TODO config file needed for pathnames + ~ didn't work? (directory not found)
CE_TOPD="/home/ben/Code"
CE_PROJECT="$(echo $1|tr [a-z] [A-Z])"

if [ -d "$CE_TOPD/$CE_PROJECT" ]
 then
	cd "$CE_TOPD/$CE_PROJECT/"

	"${EDITOR:-nano}" makefile

	if [ -f makefile~ ]				# If created a new version, move the previous to the Backup folder
	 then
#TODO should use a config setting for Backup folder location
		mv makefile~ "$CE_TOPD/Backup/${CE_PROJECT}_makefile"
#TODO Could keep several versions in Backup?
	fi

	read -p "MAKE ALL? " YN
	case $YN in
	  [Yy])
		make
		;;
	esac

	read -p "MAKE INSTALL? " YN
	case $YN in
	  [Yy])
		make install
		;;
	esac

else
	echo "Unknown project:[$CE_PROJECT]"
fi

# Don't return to initial directory to allow for make to be run

