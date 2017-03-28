#!/bin/bash
#
# Initial setup script for clice - the command line coding ecosystem
#	(will probably be repaced by GNU configure in due course)
#
# By: Andrew Bennington	2017	Licence: GPL v3+	More at: www.benningtons.net
#
# Setup:	alias to a command name in your .bashrc		i.e. edc '. ce_edit.sh'
#			use '. ' to run in the current shell to benefit from directory and flag changes (i.e. debug status)
#

echo "place .clicerc in home directory"
if [ ! -f "clicerc.template" ]
 then
    echo "ERROR - no .clicerc.template script found"
    exit
fi
cp clicerc.template "${HOME}/.clicerc"

echo "running .clicerc to set shell environment variables"
. "${HOME}/.clicerc"

if [ `grep ".clice" "${HOME}/.bashrc"|wc -l` -eq 0 ]
 then
	echo "update .bashrc to run .clicerc on each login"
	echo "# execute clice startup script" >> "${HOME}/.bashrc"
	echo ". .clicerc" >> "${HOME}/.bashrc"
else
	echo ".bashrc already set to run .clicerc on each login"
fi

echo "place .makefile.template in coding home directory ${GXT_CODE_HOME}"
if [ ! -f "makefile.template" ]
 then
    echo "ERROR - no makefile.template found"
    exit
fi
cp makefile.template "${GXT_CODE_HOME}/.makefile.template"

echo "place the project config .template.clice in coding home directory ${GXT_CODE_HOME}"
if [ ! -f "template.clice" ]
 then
    echo "ERROR - no template.clice found"
    exit
fi
cp template.clice "${GXT_CODE_HOME}/.template.clice"

exit
