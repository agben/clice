# Shell command variables
SHELL = /bin/sh
GCC = /usr/bin/gcc
CFLAGS= -D$(GXT_DEBUG) -Wall -fmax-errors=5

# Install paths according to GNU make standards
prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
includedir = $(prefix)/include
#objdir = ~/Code/Obj
objdir = $(GXT_CODE_OBJECT)

# Local object libraries
libgxtfa = $(objdir)/libgxtfa.a
libgxtnc = $(objdir)/libgxtnc.a
libgxtut = $(objdir)/libgxtut.a

#notes:
#	$@ = target
#	$^ = list of all prerequisites
#	$< = just the first prerequisite

# Executables
all: ce_clice ce_gen_make ce_scan_bash ce_scan_c ce_scan_obj ce_scan_project
ce_clice: ce_clice.c $(objdir)/ce_args.o $(objdir)/cef_main.o $(libgxtfa) $(libgxtnc)
	$(GCC) $(CFLAGS) $^ -o $@ -lmenu -lncurses -lsqlite3
ce_gen_make: ce_gen_make.c $(objdir)/ce_args.o $(objdir)/cef_main.o $(libgxtfa) $(libgxtut)
	$(GCC) $(CFLAGS) $^ -o $@ -lsqlite3
ce_scan_bash: ce_scan_bash.c $(objdir)/ce_args.o $(objdir)/cef_main.o $(libgxtfa) $(libgxtut)
	$(GCC) $(CFLAGS) $^ -o $@ -lsqlite3
ce_scan_c: ce_scan_c.c $(objdir)/ce_args.o $(objdir)/cef_main.o $(libgxtfa) $(libgxtut)
	$(GCC) $(CFLAGS) $^ -o $@ -lsqlite3
ce_scan_obj: ce_scan_obj.c $(objdir)/ce_args.o $(objdir)/cef_main.o $(libgxtfa) $(libgxtut)
	$(GCC) $(CFLAGS) $^ -o $@ -lsqlite3
ce_scan_project: ce_scan_project.c $(objdir)/ce_args.o $(objdir)/cef_main.o $(libgxtfa) $(libgxtut)
	$(GCC) $(CFLAGS) $^ -o $@ -lsqlite3

# Subroutines and functions
$(objdir)/cef_main.o: cef_main.c \
						$(includedir)/ce_main.h $(includedir)/ce_main_def.h
	$(GCC) $(CFLAGS) -c $< -o $@
$(objdir)/ce_args.o: ce_args.c
	$(GCC) $(CFLAGS) -c $< -o $@

# Tidy-up.
clean:
	-rm *~

# Install project for operational use
install: $(includedir)/ce_main.h $(includedir)/ce_main_def.h /etc/bash_completion.d/ce_complete.sh \
		$(bindir)/ce_edit.sh $(bindir)/ce_edit_make.sh $(bindir)/ce_clice $(bindir)/ce_gen_make \
		$(bindir)/ce_scan_all.sh $(bindir)/ce_scan_bash $(bindir)/ce_scan_c $(bindir)/ce_scan_obj \
		$(bindir)/ce_scan_project $(bindir)/ce_edit_project.sh
$(includedir)/ce_main.h: ce_main.h
	sudo cp $^ $@
$(includedir)/ce_main_def.h: ce_main_def.h
	sudo cp $^ $@
$(bindir)/ce_edit.sh: ce_edit.sh
	sudo cp $^ $@
$(bindir)/ce_edit_make.sh: ce_edit_make.sh
	sudo cp $^ $@
$(bindir)/ce_scan_all.sh: ce_scan_all.sh
	sudo cp $^ $@
$(bindir)/ce_edit_project.sh: ce_edit_project.sh
	sudo cp $^ $@
$(bindir)/ce_clice: ce_clice
	sudo cp $^ $@
$(bindir)/ce_gen_make: ce_gen_make
	sudo cp $^ $@
$(bindir)/ce_scan_bash: ce_scan_bash
	sudo cp $^ $@
$(bindir)/ce_scan_c: ce_scan_c
	sudo cp $^ $@
$(bindir)/ce_scan_obj: ce_scan_obj
	sudo cp $^ $@
$(bindir)/ce_scan_project: ce_scan_project
	sudo cp $^ $@
/etc/bash_completion.d/ce_complete.sh: ce_complete.sh
	sudo cp $^ $@

# Remove project from operational use
uninstall:
	sudo rm $(includedir)/ce_main.h
	sudo rm $(includedir)/ce_main_def.h
	sudo rm $(bindir)/ce_edit.sh
	sudo rm $(bindir)/ce_edit_make.sh
	sudo rm $(bindir)/ce_edit_project.sh
	sudo rm $(bindir)/ce_scan_all.sh
	sudo rm $(bindir)/ce_clice
	sudo rm $(bindir)/ce_gen_make
	sudo rm $(bindir)/ce_scan_bash
	sudo rm $(bindir)/ce_scan_c
	sudo rm $(bindir)/ce_scan_obj
	sudo rm $(bindir)/ce_scan_project
	sudo rm /etc/bash_completion.d/ce_complete.sh
