# $Id: Makefile.am,v 1.1 1999-11-01 07:23:55 knok Exp $
#

AUTOMAKE_OPTIONS = no-dependencies

INCLUDES = -I. -I../lib -I../src

lib_LTLIBRARIES = libnmz.la

libnmz_la_SOURCES = libnamazu.c libnamazu.h
libnmz_la_LIBADD = @LIBOBJS@
libnmz_la_LDFLAGS = -version-info 2:0:0
