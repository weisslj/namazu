# $Id: Makefile.am,v 1.5 2000-01-13 18:33:01 rug Exp $ 
AUTOMAKE_OPTIONS = no-dependencies

EXTRA_DIST = memcmp.c strcspn.c vsnprintf.c memmove.c strncasecmp.c \
	     memset.c strcasecmp.c strstr.c system.h easy-vsnprintf.c

noinst_LIBRARIES = libnmzut.a
libnmzut_a_SOURCES = getopt.c getopt1.c getopt.h
libnmzut_a_LIBADD = @LIBOBJS@  

