# Process this file with automake to produce Makefile.in
#
# $Id: Makefile.am,v 1.22 2000-01-03 05:32:04 rug Exp $
#

AUTOMAKE_OPTIONS = 1.4 no-dependencies

INCLUDES = -I. -I.. -I../lib -I../src -I../intl -I../nmz
localedir   = $(datadir)/locale
confdir     = $(sysconfdir)/$(PACKAGE)
indexdir    = $(localstatedir)/$(PACKAGE)/index
DEFS = -DLOCALEDIR=\"$(localedir)\" \
       -DOPT_CONFDIR=\"$(confdir)\" \
       -DOPT_INDEXDIR=\"$(indexdir)\" \
       @DEFS@

pkginclude_HEADERS = libnamazu.h codeconv.h rcfile.h field.h hlist.h \
	idxname.h parser.h re.h search.h util.h var.h wakati.h i18n.h \
	regex.h
lib_LTLIBRARIES = libnmz.la

libnmz_la_SOURCES = libnamazu.c libnamazu.h codeconv.c codeconv.h rcfile.c \
	rcfile.h field.c field.h hlist.c hlist.h \
	idxname.c idxname.h parser.c parser.h re.c re.h \
	search.c search.h seed.c seed.h util.c util.h var.c var.h \
	wakati.c wakati.h i18n.c i18n.h regex.c regex.h \
	alias.c alias.h replace.c replace.h

libnmz_la_LDFLAGS = -version-info 1:0:0
