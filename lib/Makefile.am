# $Id: Makefile.am,v 1.2 1999-10-11 04:25:10 satoru Exp $ 
AUTOMAKE_OPTIONS = no-dependencies

EXTRA_DIST = memcmp.c strcspn.c vsnprintf.c memmove.c strncasecmp.c \
	     memset.c strcasecmp.c strstr.c

noinst_LIBRARIES = libnmzut.a
libnmzut_a_SOURCES = getopt.c getopt1.c getopt.h regex.c regex.h
libnmzut_a_LIBADD = @LIBOBJS@  

