# $Id: Makefile.am,v 1.8 2000-02-14 05:05:33 rug Exp $ 
AUTOMAKE_OPTIONS = no-dependencies

EXTRA_DIST = alloca.c memcmp.c strcspn.c vsnprintf.c memmove.c strncasecmp.c \
	     memset.c strcasecmp.c strstr.c system.h easy-vsnprintf.c

noinst_LIBRARIES = libnmzut.a
libnmzut_a_SOURCES = getopt.c getopt1.c getopt.h
libnmzut_a_LIBADD = @LIBOBJS@  

