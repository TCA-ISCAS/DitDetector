#################################################################
#
#  Makefile for OIVT lo
#
#  History:
#
#    12/11/98 HSL : Created
#
#################################################################

#  Instructions:
#
#  * In the "profile" rule, edit the value of TECHINCLUDE to point to
#	the OIT headers.
#  * BUILDDIR should point to the directory that contains the makefile
#	and scclostr_u.c and scclo_u.h
#  * Note that the directories specified by TECHINCLUDE and BUILDDIR 
#	must contain Unix files, so remove CR and ctrl Zs from DOS files.
#  * It may be necessary to modify the INCLUDE and LIB variables if
#	the compiler and/or system libraries are located in different
#	directories than those specified here

SHELL=/bin/ksh

lo:
	@export TARGET="sc_lo";\
	make profile

#----------------------------------------------------------------------
#
#  Sets compiler switches for the current OS, then builds program.
#

profile:
	@export OS=`uname -s`;\
	export TECHINCLUDE=-I../common;\
	echo; echo "Building for OS: $$OS"; echo;\
	case $$OS in \
	AIX) \
		export CC='cc';\
		export LIB_FILE_EXT='o';\
		export CFLAGS='-w -qdollar -qro -qcpluscmt -DNDEBUG -D_AIX_SOURCE -DUNIX';\
		export LD='cc';\
		export LD_OPTS='-bM:SRE -bnoentry -b noquiet -bE:exp/sc_lo.exp -eEntryFunc';;\
	HP-UX) \
		export CC='cc';\
		export LIB_FILE_EXT='sl';\
		export CFLAGS='-w -Ae +ESlit +ESsfc +z +u1 PIC -DNDEBUG -D_HPUX_SOURCE -DUNIX';\
		export LD='ld';\
		export LD_OPTS='-b +s';\
		export INCLUDE='-I/usr/include/Motif1.2 -I/usr/include/X11R5';\
		export LIB='-L/usr/lib/Motif1.2 -L/usr/lib/X11R5 -lm';;\
	SunOS) \
		export CC='cc';\
		export LIB_FILE_EXT='so'; \
		export CFLAGS='-w -misalign -xCC -G -K PIC -DNDEBUG -DUNIX'; \
		export LD='cc'; \
		export LD_OPTS='-w -misalign -xCC -G -K PIC'; \
		export INCLUDE='-I/usr/openwin/include -I/usr/dt/include'; \
		export LIB='-L/usr/openwin/lib -L/usr/dt/lib -lgen -lm';; \
	SunOSi386) \
		export CC='cc';\
		export LIB_FILE_EXT='so'; \
		export CFLAGS='-w -misalign -xCC -G -K PIC -DNDEBUG -DUNIX'; \
		export LD='cc'; \
		export LD_OPTS='-w -misalign -xCC -G -K PIC'; \
		export INCLUDE='-I/usr/openwin/include -I/usr/dt/include'; \
		export LIB='-L/usr/openwin/lib -L/usr/dt/lib -lgen -lm';; \
	FreeBSD) \
		export CC='cc';\
		export LIB_FILE_EXT='so';\
		export CFLAGS='-w -shared -fPIC -DNDEBUG -DUNIX';\
		export LD='gcc';\
		export LD_OPTS='-shared -fPIC';\
		export INCLUDE='-I/usr/include/X11';\
		export LIB='-L/usr/local/lib -L/usr/X11R6/lib';;\
	LinuxS390) \
		export CC='gcc';\
		export LIB_FILE_EXT='so';\
		export CFLAGS='-w -shared -fPIC -DNDEBUG -D_LINUX_SOURCE -DUNIX';\
		export LD='gcc';\
		export LD_OPTS='-shared -fPIC';\
		export INCLUDE='-I/usr/include/X11';\
		export LIB='-L/usr/local/lib -L/usr/X11R6/lib';;\
	Linux) \
		export CC='gcc';\
		export LIB_FILE_EXT='so';\
		export CFLAGS='-w -shared -fPIC -DNDEBUG -D_LINUX_SOURCE -DUNIX';\
		export LD='gcc';\
		export LD_OPTS='-shared -fPIC';\
		export INCLUDE='-I/usr/include/X11';\
		export LIB='-L/usr/local/lib -L/usr/X11R6/lib';;\
	Linux64) \
		export CC='gcc';\
		export LIB_FILE_EXT='so';\
		export CFLAGS='-w -shared -fPIC -DNDEBUG -D_LINUX_SOURCE -DUNIX';\
		export LD='gcc';\
		export LD_OPTS='-shared -fPIC';\
		export INCLUDE='-I/usr/include/X11';\
		export LIB='-L/usr/local/lib -L/usr/X11R6/lib';;\
	esac;\
	export BUILDDIR=".";\
	if [ ! -d "$$BUILDDIR/$$OS" ]; then mkdir $$BUILDDIR/$$OS;\
	fi;\
	make $(TARGET)-lostr


#---------------------------------------------------------------------
#
#  The rule which builds lo.

$(TARGET)-lostr:  scclostr_u.cpp
	$(CC) $(CFLAGS) $(INCLUDE) $(TECHINCLUDE) -o $$BUILDDIR/scclostr_u.o -c scclostr_u.cpp
	make $(TARGET)-so

$(TARGET)-so:
	$(LD) -o libsc_lo.$(LIB_FILE_EXT) $(LD_OPTS) $$BUILDDIR/scclo.o $$BUILDDIR/scclostr_u.o $(INCLUDE) $(TECHINCLUDE)
	@if [ -a "libsc_lo.$(LIB_FILE_EXT)" ]; then mv libsc_lo.$(LIB_FILE_EXT) $$BUILDDIR/$$OS/; fi;\
	echo; echo "Binary 'libsc_lo.$(LIB_FILE_EXT)' has been placed in $$BUILDDIR/$$OS/"; echo;
