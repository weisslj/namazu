# Process this file with automake to produce Makefile.in
#
# $Id: Makefile.am,v 1.7 1999-11-08 05:43:37 rug Exp $
#

AUTOMAKE_OPTIONS = no-dependencies

localedir   = $(datadir)/locale
confdir     = $(sysconfdir)/$(PACKAGE)
indexdir    = $(localstatedir)/$(PACKAGE)/index

INCLUDES = -I.. -I../lib -I../src -I../intl
DEFS = -DLOCALEDIR=\"$(localedir)\" \
       -DOPT_CONFDIR=\"$(confdir)\" \
       -DOPT_INDEXDIR=\"$(indexdir)\" \
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
