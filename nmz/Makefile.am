# Process this file with automake to produce Makefile.in
#
# $Id: Makefile.am,v 1.14 1999-11-19 08:08:59 satoru Exp $
#

AUTOMAKE_OPTIONS = no-dependencies

INCLUDES = -I. -I.. -I../lib -I../src -I../intl -I../nmz
localedir   = $(datadir)/locale
confdir     = $(sysconfdir)/$(PACKAGE)
indexdir    = $(localstatedir)/$(PACKAGE)/index
DEFS = -DLOCALEDIR=\"$(localedir)\" \
       -DOPT_CONFDIR=\"$(confdir)\" \
       -DOPT_INDEXDIR=\"$(indexdir)\" \
       @DEFS@

pkginclude_HEADERS = libnamazu.h codeconv.h conf.h critical.h field.h hlist.h \
	idxname.h magic.h parser.h re.h search.h util.h var.h wakati.h i18n.h \
	regex.h mode.h
lib_LTLIBRARIES = libnmz.la

libnmz_la_SOURCES = libnamazu.c libnamazu.h codeconv.c codeconv.h conf.c \
	conf.h critical.h field.c field.h hlist.c hlist.h \
	idxname.c idxname.h magic.h parser.c parser.h re.c re.h \
	search.c search.h seed.c util.c util.h var.c var.h \
	wakati.c wakati.h i18n.c i18n.h regex.c regex.h mode.c mode.h

libnmz_la_LDFLAGS = -version-info 0:0:0
