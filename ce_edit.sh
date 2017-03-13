#!/bin/bash
#
# Universal editing and source code management procedure for clice - the command line coding ecosystem
#
# By: Andrew Bennington		Licence: GPL v3+	More at: www.benningtons.net
#
# Setup:	alias to a command name in your .bashrc		i.e. edc '. ce_edit.sh'
#			use '. ' to run in the current shell to benefit from directory and flag changes (i.e. debug status)
#			see ce_setup.sh for clice setup details
#
#	clice coding directory structure
#	--------------------------------
#                   Code                 [Coding root directory]
#                    |
#           +--------+------+-------+
#           |        |      |       |
#           |      Backup   |       |    [Backup directory for previous versions of edited files]
#           |              Obj      |    [Obj directory to hold compiled objects and object libraries]
#          CE                      FA    [Project directories, each using 2 capital letters]
#           |
#   +-------+-------+
#   |               |
# ce_clice.c      ce_edit.sh             [Project source files, each starting with the project prefix in lowercase]
#
#
# Usage:	edc			change your working directory to your coding root directory
#		edc ce			change your working directory to the CE project
#		edc ce_edit.sh		change your working directory and give you options to edit, compile, etc... the given filename
#					It will be easier to provide the full filename if ce_complete.sh is used for bash completion
#						.i.e. partially type a name and press tab to complete it


cd "$GXT_CODE_HOME"
if [ $# -eq 0 ]
 then							# and stay there if nothing else entered
	return
fi

CE_FNAM=$1						# filename entered
CE_PROJECT="$(echo ${CE_FNAM:0:2}|tr [a-z] [A-Z])"	# project prefix

if [ ! -d "${CE_PROJECT}" ]
 then
    echo "--Project ${CE_PROJECT} not found--"
    return
fi

cd "${CE_PROJECT}/"				# relocate to the project directory
if [ ${#CE_FNAM} -eq 2 ]
 then							# and stay there if nothing else entered
	return
fi

if [ ! -f "$CE_FNAM" ]
 then
    echo "--Filename ${CE_FNAM} not found--"
    return
fi

CE_LANG="$(echo ${CE_FNAM#*.}|tr [a-z] [A-Z])"		# determine source language from file extension

# Check file extension against supported languages
case "$CE_LANG" in
# F)				# FORTRAN. The .f extension assumes fixed format whereas .f95 is free format
# ;;
 [CH])			# C. The .c for program code and the .h for included library files
 ;;
 TXT)			# text files. Usually documentation notes
 ;;
 SH)			# Shell scripts.
 ;;
 *) echo "--unknown language extention [${CE_LANG}]--"
	return
 ;;
esac

CE_PROGN="${CE_FNAM%.*}"		# program name minus its extension

read -p "EDIT (Y/T) " CE_EDD
case $CE_EDD in
  [Yy])
	"${EDITOR:-nano}" $CE_FNAM			# See /usr/share/nano for syntax highlighting rules

	if [ -f "$CE_FNAM"~ ]				# If created a new version, move the previous to the Backup folder
	 then
		case "$CE_LANG" in
		 [CH])							# .c c programs or .h c library files
			ctags --c-kinds=f -x $CE_FNAM > $CE_PROGN.ce	# scan source code for C symbols
			ce_scan_c $CE_FNAM			# Update clice to show new version created
			rm $CE_PROGN.ce
		 ;;
		 TXT)							# ignore text files
		 ;;
		 SH)							# Shell scripts.
			ce_scan_bash $CE_FNAM		# Update clice to show new version created
		 ;;
		esac

		mv *~ "${GXT_CODE_BACKUP}/"			# using wildcard (*~) to keep folders tidy
#TODO Could keep several versions in Backup?
	fi
	;;
  [Tt])
	touch $CE_FNAM						# Just touch file (update last modified) so make picks it up
	;;
esac

case "$CE_LANG" in
 [FC])									# Requires compiling?
	read -p "COMPILE (Y/d/D) " CE_CO
	if [[ $CE_CO == "d" ]]
	 then
		GXT_DEBUG="DEBUG=1"
		CE_CO="Y"
	else
		if [[ $CE_CO == "D" ]]
		 then
			GXT_DEBUG="DEBUG=2"			# trigger debug messages and sleep for $DEBUG-1 seconds
			CE_CO="Y"
		else
			GXT_DEBUG="DEBUG=0"
		fi
	fi
#	export GXT_DEBUG					# export for use in subsequent makefiles
 ;;
 *)
	CE_CO="N"
 ;;
esac

CE_ONAM="${GXT_CODE_OBJECT}/${CE_PROGN}.o"		# object files full name

case $CE_CO in
 [Yy])
	case "$CE_LANG" in
#	  F)					# Fortran complier options
							# -c = compile and assemble but don't link
							# -fmax-errros = limit number of compile errors displayed
							# -fno-underscoring = don't tag an underscore to the end of object
							#	filenames. It prevents linking to C objects
							# -I = where INCLUDE will look for files
							# -L = location of my libraries
							# -l = libraries to link against
							# -Wall = I want to see all warnings
#TODO need a test for a program or subroutine/function
#			gfortran	\
#					-c	\
#					-Wall	\
#					-Wtabs	\
#					-L "/home/ben/Code/Obj" \
#					-fmax-errors=5 \
#					-fno-underscoring \
#					$CE_FNAM	\
#					-lrt \
#					-lsubs \
#					-lsqlite3 \
#					-o $CE_ONAM
#    ;;

	  C)					# C complier options
							# -Wall = I want to see all warnings
							# -o = output file
							# -l = libraries to compile against
		gcc \
			-D$GXT_DEBUG \
			-c	\
			-Wall	\
			$CE_FNAM	\
			-o $CE_ONAM
     ;;
	esac

	if [ -f "$CE_ONAM" ]							# sucessfully created an object file?
	 then
#		nm -t d -f posix $CE_ONAM >> $CE_PROGN.ce	# Analyse obj file read for CE
		objdump -rtl $CE_ONAM > $CE_PROGN.ce		# Analyse obj file read for CE
		ce_scan_obj $CE_PROGN						# Scan links to other modules for CE
		rm $CE_PROGN.ce
	fi
 ;;
esac

