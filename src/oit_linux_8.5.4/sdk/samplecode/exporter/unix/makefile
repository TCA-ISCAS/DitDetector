# MAKEFILE FOR EXPORTER EXECUTABLE
#
# C EXPORT SDKS
# EXECUTABLE MODULE FOR JAVA INTERFACE.
#
# ORACLE SAMPLE APPLICATION
# Copyright (c) 2001, 2016, Oracle and/or its affiliates. All rights reserved.
#
# ORACLE PROVIDES THIS CODE AS IS AND PROVIDES NO WARRANTIES
# OR GUARANTEES OF ANY KIND.  YOU ARE FREE TO USE, MODIFY,
# REPRODUCE, AND DISTRIBUTE THIS CODE AND/OR ANY MODIFIED VERSION
# THERE OF SO LONG AS YOU ACKNOWLEDGE THAT ORACLE HAS NO WARRANTY
# OBLIGATIONS OR LIABILITY FOR SUCH USE AND SO LONG AS SUCH USE IS
# IN ACCORDANCE WITH THE TERMS AND CONDITIONS OF AN EXISITING AND VALID
# LICENSE AGREEMENT BETWEEN YOU AND ORACLE

INCLUDE_DIR = -I../../../../common
LIBRARY_DIR = -L../../../..

FLAGS       = -O2 -DDISABLE_REPORTING
CC          = gcc
CFLAGS      = $(STATIC) $(FLAGS) $(DEBUG) $(INCLUDE_DIR) 

LIBRARIES   = -lsc_ex -lsc_da 

all:: exporter

exporter: cfg.h cfg.c exporter.c
	@OS=`uname -s`;\
	export OS;\
	case $$OS in \
	HP-UX) \
		RUNPATH='-Xlinker +b.';\
		export RUNPATH;;\
	SunOS) \
		RUNPATH='-Xlinker -R -Xlinker .';\
		export RUNPATH;;\
	Linux) \
		RUNPATH='-Xlinker --rpath -Xlinker . -Xlinker --rpath -Xlinker ../../../..';\
		export RUNPATH;;\
	AIX) \
		RUNPATH='';\
		export RUNPATH;;\
	esac; \
	$(CC) exporter.c -o exporter $(CFLAGS) $(LIBRARY_DIR) $(LIBRARIES) $$RUNPATH


.PHONY: clobber clean

clobber: clean
clean:
	-/usr/bin/rm -f exporter

