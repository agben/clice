#!/bin/bash
#
# project config editor for clice (CE) - the command line coding ecosystem
#
# Setup:	alias to a command name in your .bashrc
#			i.e. edo '. ce_edit_project.sh'
#			use '. ' to run in the current shell to benefit from directory and flag changes
#			update and run ce_setup.sh to configure clice settings
# Usage:	edp ce
#			this will change your directory to CE (project name) and open the .ce.clice config file
#
# Parameter 1 is the project name
#

CE_PROJECT="$(echo $1|tr [a-z] [A-Z])"
CE_CLICEFILE=".$(echo $CE_PROJECT|tr [A-Z] [a-z]).clice"

if [ -z $CE_PROJECT ]
 then
	cd "${GXT_CODE_HOME}"
	"${EDITOR:-nano}" .master.clice

elif [ -d "${GXT_CODE_HOME}/${CE_PROJECT}" ]
 then
	cd "${GXT_CODE_HOME}/${CE_PROJECT}/"

	if [ ! -f "$CE_CLICEFILE" ]					# check if the hidden config file exists
	 then
#TODO should ask for confirmation here before using a template
		cp "${GXT_CODE_HOME}/.template.clice" "${CE_CLICEFILE}"	# If no clice config file for this project then start with a template
	fi

	"${EDITOR:-nano}" "$CE_CLICEFILE"

	if [ -f "$CE_CLICEFILE~" ]				# If created a new version, move the previous to the Backup folder
	 then
		mv "${CE_CLICEFILE}~" "${GXT_CODE_BACKUP}/${CE_CLICEFILE}"
#TODO Could keep several versions in Backup?
		ce_scan_project
	fi

else
	echo "Unknown project:[$CE_PROJECT]"
fi
