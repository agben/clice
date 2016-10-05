#!/bin/bash
#
# Universal editing and source code management procedure for clice (CE) - the command line coding ecosystem
#
# Setup:	alias to a command name in your .bashrc
#			i.e. edc '. ce_edit.sh'
#			use '. ' to run in the current shell to benefit from directory and flag changes (i.e. debug status)
# Usage:	edc ce_clice.c
#			this will change your directory to CE (project name) and give you options to edit, compile, etc...
#
# Parameter 1 is the source code full filename+extension e.g. example.c
#			This should be easierto enter if ce_complete.sh is used for bash completion .i.e. partially type a name and press tab to complete it

#TODO CE_DBUG compiler flags will need to be switched on/off in makefile or have a final make question that can force it?

CE_FNAM=$1
CE_PROGN="${CE_FNAM%.*}"
#TODO config file needed for pathnames + ~ didn't work? (directory not found)
CE_ONAM="/home/ben/Code/Obj/${CE_PROGN}.o"
CE_LANG="$(echo ${CE_FNAM#*.}|tr [a-z] [A-Z])"
CE_PROJECT="$(echo ${CE_FNAM:0:2}|tr [a-z] [A-Z])"
CE_DNAME=$PWD

# Check file extension against supported languages
case "$CE_LANG" in
# F)				# FORTRAN. The .f extension assumes fixed format whereas .f95 is free format
# ;;
 [CH])			# C. The .c for program code and the .h for include library files
 ;;
 TXT)			# text files. Usually documentation notes
 ;;
 SH)			# Shell scripts.
 ;;
 *) echo "!!!!!unknown language extention = [${CE_LANG}]!!!!!"
    return
 ;;
esac

if [ ! -d "/home/ben/Code/${CE_PROJECT}" ]
 then
    echo "!!Cannot find project!!"
    return
fi

cd /home/ben/Code/${CE_PROJECT}/

if [ ! -f "$CE_FNAM" ]
 then
    echo "!!Cannot find source file?!!"
    cd "$CE_DNAME"
    return
fi

read -p "EDIT (Y/N/T) " CE_EDD
case $CE_EDD in
  [Yy])
	nano $CE_FNAM						# See /usr/share/nano for syntax highlighting rules

	if [ -f "$CE_FNAM"~ ]				# If created a new version, move the previous to the Backup folder
	 then
		if [ $CE_LANG != "TXT" ]
		 then
			ce_scan $CE_PROGN --language $CE_LANG		# Update clice to show new version created
		fi
#TODO should use a config setting for Backup folder location
		mv *~ ~/Code/Backup/				# using wildcard (*~) to keep folders tidy
#TODO Could keep several versions in Backup?
	fi
	;;
  [Tt])
	touch $CE_FNAM						# Just touch file (update last modified) so make picks it up
	;;
esac

case "$CE_LANG" in
 [FC])									# Requires compiling?
	read -p "COMPILE (Y/N/d/D) " CE_CO
	if [[ $CE_CO == "d" ]]
	 then
		CE_DBUG="DEBUG=1"
		CE_CO="Y"
	else
		if [[ $CE_CO == "D" ]]
		 then
			CE_DBUG="DEBUG=2"			# trigger debug messages and sleep for $DEBUG-1 seconds
			CE_CO="Y"
		else
			CE_DBUG="DEBUG=0"
		fi
	fi
	export CE_DBUG						# export for use in subsequent makefiles
 ;;
 *)
	CE_CO="N"
 ;;
esac

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
							# -Wtabs = but don't want to have to convert FWS tabs to spaces
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
			-D$CE_DBUG \
			-c	\
			-Wall	\
			$CE_FNAM	\
			-o $CE_ONAM
     ;;
	esac

	if [ -f "$CE_ONAM" ]							# sucessfully created an object file?
	 then
		nm -t d -f posix $CE_ONAM >> $CE_PROGN.ce	# Analyse obj file read for CE
		ce_scan_obj $CE_PROGN						# Scan links to other modules for CE
		rm $CE_PROGN.ce
	fi
 ;;
esac

cd "$DNAM"						# return to original folder
