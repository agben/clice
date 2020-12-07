#!/bin/bash
#
# makefile editor for clice (CE) - the command line coding ecosystem
#
# Setup:	alias to a command name in your .bashrc
#			i.e. edm '. ce_edit_make.sh'
#			use '. ' to run in the current shell to benefit from directory and flag changes
#			edit ce_setup.sh to change clice config settings
# Usage:	edm ce
#			this will change your directory to CE (project name) and open the makefile
#
# Parameter 1 is the project name
#

CE_PROJECT="$(echo $1|tr [a-z] [A-Z])"

if [ -d "${GXT_CODE_HOME}/${CE_PROJECT}" ]
 then
	cd "${GXT_CODE_HOME}/${CE_PROJECT}/"

	read -p "EDIT MAKEFILE? " YN
	case $YN in
	  [Yy])
		"${EDITOR:-nano}" makefile

		if [ -f makefile~ ]				# If created a new version, move the previous to the Backup folder
		 then
			mv makefile~ "${GXT_CODE_BACKUP}/${CE_PROJECT}_makefile"
#TODO Could keep several versions in Backup?
		fi
		;;
	esac

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

