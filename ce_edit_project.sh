#!/bin/bash
#
# project config editor for clice (CE) - the command line coding ecosystem
#
# Setup:	alias to a command name in your .bashrc
#			i.e. edo '. ce_edit_project.sh'
#			use '. ' to run in the current shell to benefit from directory and flag changes
# Usage:	edp ce
#			this will change your directory to CE (project name) and open the .ce.clice config file
#
# Parameter 1 is the project name
#

#TODO config file needed for pathnames + ~ didn't work? (directory not found)
CE_TOPD="/home/ben/Code"
CE_PROJECT="$(echo $1|tr [a-z] [A-Z])"
CE_CLICEFILE=".$(echo $CE_PROJECT|tr [A-Z] [a-z]).clice"

if [ -z $CE_PROJECT ]
 then
	cd "$CE_TOPD"
	"${EDITOR:-nano}" .master.clice

elif [ -d "$CE_TOPD/$CE_PROJECT" ]
 then
	cd "$CE_TOPD/$CE_PROJECT/"

	if [ ! -f "$CE_CLICEFILE" ]					# check if the hidden config file exists
	 then
#TODO should ask for confirmation here before using a template
		cp "$CE_TOPD/.template.clice" "$CE_CLICEFILE"	# If no clice config file for this project then start with a template
	fi

	"${EDITOR:-nano}" "$CE_CLICEFILE"

	if [ -f "$CE_CLICEFILE~" ]				# If created a new version, move the previous to the Backup folder
	 then
#TODO should use a config setting for Backup folder location
		mv "$CE_CLICEFILE~" "$CE_TOPD/Backup/$CE_CLICEFILE"
#TODO Could keep several versions in Backup?
		ce_scan_project
#TODO Here we can scan the new config file and update the clice db
	fi

else
	echo "Unknown project:[$CE_PROJECT]"
fi

# Don't return to initial directory to allow for make to be run
