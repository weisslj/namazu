# $Id: Makefile.am,v 1.11 2000-08-25 09:52:58 rug Exp $ 
AUTOMAKE_OPTIONS = 1.4 no-dependencies

EXTRA_DIST = alloca.c memcmp.c strcspn.c vsnprintf.c memmove.c strncasecmp.c \
	     memset.c strcasecmp.c strstr.c system.h easy-vsnprintf.c

noinst_LTLIBRARIES = libnmzut.la
libnmzut_la_SOURCES = getopt.c getopt1.c getopt.h
libnmzut_la_LIBADD = @LIBOBJS@  
libnmzut_la_LDFLAGS = -static
