# This makefile has been generated by clice - the command line coding ecosystem
#	more details at www.benningtons.net
# built on 14042017 at 2325

# Shell command variables
SHELL = /bin/sh
GCC = /usr/bin/gcc
CFLAGS= -D$(GXT_DEBUG) -std=gnu11 -Wall -fmax-errors=5

# Install paths according to GNU make standards
prefix = /usr/local
bindir = $(prefix)/bin
includedir = $(prefix)/include
completedir = /etc/bash_completion.d
objdir = $(GXT_CODE_OBJECT)

# This project's executable programs
all:	\
	ce_clice ce_gen_make ce_scan_bash ce_scan_c ce_scan_obj ce_scan_project 

# Tidy-up.
clean:
	-rm *~

# Install project for operational use
install:	\
	$(includedir)/ce_main.h $(includedir)/ce_main_def.h $(bindir)/ce_clice $(completedir)/ce_complete.sh  \
	$(bindir)/ce_edit.sh $(bindir)/ce_edit_make.sh $(bindir)/ce_edit_project.sh $(bindir)/ce_gen_make  \
	$(bindir)/ce_scan_all.sh $(bindir)/ce_scan_bash $(bindir)/ce_scan_c $(bindir)/ce_scan_obj  \
	$(bindir)/ce_scan_project $(bindir)/ce_setup.sh 
$(includedir)/ce_main.h: ce_main.h
	sudo cp $^ $@
$(includedir)/ce_main_def.h: ce_main_def.h
	sudo cp $^ $@
$(bindir)/ce_clice: ce_clice
	sudo cp $^ $@
$(completedir)/ce_complete.sh: ce_complete.sh
	sudo cp $^ $@
$(bindir)/ce_edit.sh: ce_edit.sh
	sudo cp $^ $@
$(bindir)/ce_edit_make.sh: ce_edit_make.sh
	sudo cp $^ $@
$(bindir)/ce_edit_project.sh: ce_edit_project.sh
	sudo cp $^ $@
$(bindir)/ce_gen_make: ce_gen_make
	sudo cp $^ $@
$(bindir)/ce_scan_all.sh: ce_scan_all.sh
	sudo cp $^ $@
$(bindir)/ce_scan_bash: ce_scan_bash
	sudo cp $^ $@
$(bindir)/ce_scan_c: ce_scan_c
	sudo cp $^ $@
$(bindir)/ce_scan_obj: ce_scan_obj
	sudo cp $^ $@
$(bindir)/ce_scan_project: ce_scan_project
	sudo cp $^ $@
$(bindir)/ce_setup.sh: ce_setup.sh
	sudo cp $^ $@

# Remove project from operational use
uninstall:
	sudo rm $(includedir)/ce_main.h
	sudo rm $(includedir)/ce_main_def.h
	sudo rm $(bindir)/ce_clice
	sudo rm $(completedir)/ce_complete.sh
	sudo rm $(bindir)/ce_edit.sh
	sudo rm $(bindir)/ce_edit_make.sh
	sudo rm $(bindir)/ce_edit_project.sh
	sudo rm $(bindir)/ce_gen_make
	sudo rm $(bindir)/ce_scan_all.sh
	sudo rm $(bindir)/ce_scan_bash
	sudo rm $(bindir)/ce_scan_c
	sudo rm $(bindir)/ce_scan_obj
	sudo rm $(bindir)/ce_scan_project
	sudo rm $(bindir)/ce_setup.sh

# Functions and their dependencies

ce_clice: ce_clice.c $(objdir)/ce_args.o $(includedir)/ce_main.h $(objdir)/cef_main.o \
	 $(includedir)/fa_def.h $(objdir)/libgxtnc.a $(includedir)/nc_main.h $(includedir)/ut_error.h \
	 $(includedir)/ce_main_def.h $(objdir)/libgxtfa.a $(includedir)/fa_sql_def.h $(includedir)/fa_lun.h \
	 
	$(GCC) $(CFLAGS) $^ -o $@ -lncurses -lmenu -lsqlite3
ce_gen_make: ce_gen_make.c $(objdir)/ce_args.o $(includedir)/ce_main.h $(objdir)/cef_main.o \
	 $(includedir)/fa_def.h $(includedir)/ut_date.h $(objdir)/libgxtut.a $(includedir)/ut_error.h \
	 $(includedir)/ce_main_def.h $(objdir)/libgxtfa.a $(includedir)/fa_sql_def.h $(includedir)/fa_lun.h \
	 
	$(GCC) $(CFLAGS) $^ -o $@ -lsqlite3
ce_scan_bash: ce_scan_bash.c $(objdir)/ce_args.o $(includedir)/ce_main.h $(objdir)/cef_main.o \
	 $(includedir)/fa_def.h $(includedir)/ut_date.h $(objdir)/libgxtut.a $(includedir)/ut_error.h \
	 $(includedir)/ce_main_def.h $(objdir)/libgxtfa.a $(includedir)/fa_sql_def.h $(includedir)/fa_lun.h \
	 
	$(GCC) $(CFLAGS) $^ -o $@ -lsqlite3
ce_scan_c: ce_scan_c.c $(objdir)/ce_args.o $(includedir)/ce_main.h $(objdir)/cef_main.o \
	 $(includedir)/fa_def.h $(includedir)/ut_date.h $(objdir)/libgxtut.a $(includedir)/ut_error.h \
	 $(includedir)/ce_main_def.h $(objdir)/libgxtfa.a $(includedir)/fa_sql_def.h $(includedir)/fa_lun.h \
	 
	$(GCC) $(CFLAGS) $^ -o $@ -lsqlite3
ce_scan_obj: ce_scan_obj.c $(objdir)/ce_args.o $(includedir)/ce_main.h $(objdir)/cef_main.o \
	 $(includedir)/fa_def.h $(includedir)/ut_date.h $(objdir)/libgxtut.a $(includedir)/ut_error.h \
	 $(includedir)/ce_main_def.h $(objdir)/libgxtfa.a $(includedir)/fa_sql_def.h $(includedir)/fa_lun.h \
	 
	$(GCC) $(CFLAGS) $^ -o $@ -lsqlite3
ce_scan_project: ce_scan_project.c $(objdir)/ce_args.o $(includedir)/ce_main.h $(objdir)/cef_main.o \
	 $(includedir)/fa_def.h $(includedir)/ut_date.h $(objdir)/libgxtut.a $(includedir)/ut_error.h \
	 $(includedir)/ce_main_def.h $(objdir)/libgxtfa.a $(includedir)/fa_sql_def.h $(includedir)/fa_lun.h \
	 
	$(GCC) $(CFLAGS) $^ -o $@ -lsqlite3
$(objdir)/ce_args.o: ce_args.c $(includedir)/ce_main.h 
	$(GCC) $(CFLAGS) -c $< -o $@
$(objdir)/cef_main.o: cef_main.c $(includedir)/ce_main.h $(includedir)/ce_main_def.h \
	 $(includedir)/fa_def.h $(objdir)/libgxtfa.a $(includedir)/ut_error.h 
	$(GCC) $(CFLAGS) -c $< -o $@
