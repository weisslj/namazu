# Process this file with automake to produce Makefile.in
#
# $Id: Makefile.am,v 1.6 1999-11-08 05:06:03 knok Exp $
#

AUTOMAKE_OPTIONS = no-dependencies

INCLUDES = -I.. -I../lib -I../src -I../intl
DEFS = -DLOCALEDIR=\"$(localedir)\" \
       -DOPT_INDEXDIR=\"${localstatedir}/${PACKAGE}/index\" \
       -DOPT_CONFDIR=\"${sysconfdir}/${PACKAGE}\" \
       @DEFS@

pkginclude_HEADERS = libnamazu.h codeconv.h conf.h critical.h field.h hlist.h \
	idxname.h magic.h parser.h re.h regex.h search.h util.h var.h wakati.h
lib_LTLIBRARIES = libnmz.la

libnmz_la_SOURCES = libnamazu.c libnamazu.h codeconv.c codeconv.h conf.c \
	conf.h critical.h field.c field.h hlist.c hlist.h \
	idxname.c idxname.h magic.h parser.c perser.h re.c re.h \
	regex.c regex.h search.c search.h seed.c util.c util.h \
	var.c var.h wakati.c wakati.h
libnmz_la_LDFLAGS = -version-info 0:0:0
