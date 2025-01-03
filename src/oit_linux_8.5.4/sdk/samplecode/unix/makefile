#################################################################
#
#  Makefile for OI Image Export Unix Sample Applications
#     NOTE:  This makefile uses a shell script called 'configure', and will
#            not work without this script.
#
#  Copyright (c) 2001, 2014, Oracle and/or its affiliates. All rights reserved.
#################################################################

#  The default behavior is to make everything.

all:	 ixanno ixsample exsimple exredir exporter ixmulti extract_archive

.PHONY: all

# In order to make the "profile" rule reusable by all targets, we
# need this extra layer of abstraction.

ixanno:
	@TARGET="ixanno" \
	TARGETDEPS="-lsc_ex -lsc_da -lsc_ca" \
    ./configure

ixsample:
	@TARGET="ixsample" \
	TARGETDEPS="-lsc_ex -lsc_da" \
    ./configure

exsimple:
	@TARGET="exsimple" \
	TARGETDEPS="-lsc_ex -lsc_da" \
    ./configure

exredir:
	@TARGET="exredir" \
	TARGETDEPS="-lsc_ex -lsc_da" \
    ./configure

exporter:
	@TARGET="exporter" \
	TARGETDEPS="-lsc_ex -lsc_da" \
    ./configure

ixmulti:
	@TARGET="ixmulti" \
	APPFLAGS="-DIXMULTI" \
	TARGETDEPS="-lsc_ex -lsc_da" \
    ./configure
    
extract_archive:
	@TARGET="extract_archive" \
	TARGETDEPS="-lsc_da" \
    ./configure	    
