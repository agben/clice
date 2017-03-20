#!/bin/bash
#
# Scan all files for details to add to the clice database. clice - the command line coding ecosystem
#
# Run this script in each directory you wish to load (a recursive approach risks capturing unwanted files)
#
# By: Andrew Bennington		Licence: GPL v3+	More at: www.benningtons.net
#

echo "Loading project definitions"
for ce_file in .*.clice; do
	if [ -f ${ce_file} ]; then
		ce_scan_project ${ce_file}
	fi
done

echo "Loading scripts"
for ce_file in *.sh; do
	if [ -f ${ce_file} ]; then
		ce_scan_bash ${ce_file}
	fi
done

echo "Loading C header files"
for ce_file in *.h; do
	if [ -f ${ce_file} ]; then
		ce_scan_c ${ce_file}
	fi
done

echo "Loading C functions"
for ce_file in *.c; do
	if [ -f ${ce_file} ]; then
		ce_temp="${ce_file%.*}.ce"				# temp file for scan data
		ctags --c-kinds=f -x ${ce_file} > ${ce_temp}
		ce_scan_c ${ce_file}
		rm ${ce_temp}

		ce_object="${ce_file%.*}.o"				# create temp object files in current directory to avoid triggering make
		gcc \
			-fmax-errors=1 \
			-c	\
			${ce_file}	\
			-o ${ce_object}

		if [ -f ${ce_object} ]; then			# sucessfully created an object file?
			objdump -rtl ${ce_object} > ${ce_temp}
			ce_scan_obj ${ce_file%.*}
			rm ${ce_temp}
			rm ${ce_object}
		fi
	fi
done

exit
