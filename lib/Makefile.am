# $Id: Makefile.am,v 1.4 1999-11-28 23:50:31 satoru Exp $ 
AUTOMAKE_OPTIONS = no-dependencies

EXTRA_DIST = memcmp.c strcspn.c vsnprintf.c memmove.c strncasecmp.c \
	     memset.c strcasecmp.c strstr.c system.h

noinst_LIBRARIES = libnmzut.a
libnmzut_a_SOURCES = getopt.c getopt1.c getopt.h
libnmzut_a_LIBADD = @LIBOBJS@  

