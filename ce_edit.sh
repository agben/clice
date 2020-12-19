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
# Usage:edc				change your working directory to your coding root directory
#		edc ce			change your working directory to the CE project
#		edc ce_edit.sh		change your working directory and give you options to edit, compile, etc... the given filename
#						It will be easier to provide the full filename if ce_complete.sh is used for bash completion
#						.i.e. partially type a name and press tab to complete it
#
# Notes:
#	Assembly	-	As I'm currently writing assembly for an independent OS then all outputs are binary and held in the source folder


cd "$GXT_CODE_HOME"									# move to the clice coding home directory
if [ $# -eq 0 ]
 then												# and stay there if nothing else entered
	return
fi

CE_FNAM=$1											# filename passed as 1st argument
CE_PROJECT="$(echo ${CE_FNAM:0:2}|tr [a-z] [A-Z])"	# project prefix is the 1st 2 characters of the filename

if [ ! -d "${CE_PROJECT}" ]
 then
    echo "--Project ${CE_PROJECT} not found--"
    return
fi

cd "${CE_PROJECT}/"									# relocate to the project directory
if [ ${#CE_FNAM} -eq 2 ]
 then												# and stay there if nothing else entered
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
# F)			# FORTRAN. The .f extension assumes fixed format whereas .f95 is free format
# ;;
 ASM|HSM)		# assembly source and header files.
 ;;
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

CE_PROGN="${CE_FNAM%.*}"				# program name minus its extension

read -p "EDIT (Y/T) " CE_EDD
case $CE_EDD in
  [Yy])
	"${EDITOR:-nano}" $CE_FNAM			# Use prefered editor else default to nano if $EDITOR not defined

	if [ -f "$CE_FNAM"~ ]				# Created a new version of the source file? (old version saved ending with ~)
	 then								# 	some editors need configuring to create these backup files when changes have been made
		 case "$CE_LANG" in				# scan new source file for changes to update clide db
		 ASM|HSM)						# assembly source or header files.
			ce_scan_asm $CE_FNAM		# Update clice with source code details
		 ;;
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

		for CE_FILE in *~; do			# move the previous version to the Backup folder
										# using wildcard (*~) to keep folders tidy
			mv ${CE_FILE} "${GXT_CODE_BACKUP}/${CE_FILE}$(date +%Y%m%d%H%M)"	#tagging each file with a date and time stamp

			i=0							# purge older versions - set number to keep with GXT_BACKUP_VERSIONS (default=10)
			for CE_BACKUP in `ls -r ${GXT_CODE_BACKUP}/${CE_FILE}*`; do
				let i=i+1
				if [ $i -gt ${GXT_BACKUP_VERSIONS:-10} ]
				 then
					rm ${CE_BACKUP}
				fi
			done
			
		done																	# so all previous versions can be kept
	fi
	if [ -f \#* ]
	 then
		mv \#*\# "${GXT_CODE_BACKUP}/"	# move any quit and unsaved edits (emacs) to keep folders tidy
	fi
	;;
  [Tt])
	touch $CE_FNAM						# Just touch file (update last modified) so make picks it up
	;;
esac

case "$CE_LANG" in
 ASM)									# Requires assembling?
	read -p "ASSEMBLE (Y) " CE_CO
 ;;
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
	export GXT_DEBUG					# export for use in subsequent makefiles
 ;;
 *)
	CE_CO="N"
 ;;
esac

case $CE_CO in
 [Yy])
	case "$CE_LANG" in
	  ASM)					# Assembler options
		CE_ONAM="${GXT_CODE_OBJECT}/${CE_PROGN}.bin"		# binary output filename
							# -f = output format. bin = flat binary file
							# -w = warnings. I want to see all warnings
							# -o = output file
							# -MD $CE_PROGN.ce = generate makefile dependencies
		nasm \
			-o $CE_ONAM \
			-f bin \
			-w+all	\
			$CE_FNAM
	  ;;

	  C)					# C complier options
		CE_ONAM="${GXT_CODE_OBJECT}/${CE_PROGN}.o"		# object files full name
							# -c = compile and assemble but don't link
							# -D = debug option
							# -std = standard of C applied - GNU 2011
							# -fPIC = position independent code, needed if putting code into libraries
							# -Wall = I want to see all warnings
							# -o = output file
		gcc \
			-D$GXT_DEBUG \
			-c	\
			-std=gnu11 \
			-fPIC	\
			-Wall	\
			$CE_FNAM	\
			-o $CE_ONAM
     ;;
	esac


	if [ -f "$CE_ONAM" ]							# sucessfully created an object file?
	 then
		case "$CE_LANG" in
#		  ASM)										# Assembler object file analysis
													# objdump options
													# -b = object file format
													# -r = show code relocation entries
													# -s = show the symbolic table
													# -l = label output with line numbers
#			objdump -b binary -rtl $CE_ONAM > $CE_PROGN.ce	# Analyse obj file read for clice
#		  ;;
		  C)										# C object file analysis
													# objdump options
													# -r = show code relocation entries
													# -s = show the symbolic table
													# -l = label output with line numbers
			objdump -rtl $CE_ONAM > $CE_PROGN.ce	# Analyse obj file read for clice
			ce_scan_obj $CE_PROGN					# Scan links to other modules for clice
			rm $CE_PROGN.ce
		  ;;
		esac
	fi
 ;;
esac

