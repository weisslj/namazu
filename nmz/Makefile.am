# Process this file with automake to produce Makefile.in
#
# $Id: Makefile.am,v 1.24 2000-01-09 13:00:37 satoru Exp $
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

pkginclude_HEADERS = codeconv.h field.h hlist.h i18n.h idxname.h \
	libnamazu.h rcfile.h parser.h query.h re.h regex.h \
	search.h util.h var.h wakati.h 
lib_LTLIBRARIES = libnmz.la

libnmz_la_SOURCES = alias.c alias.h codeconv.c codeconv.h rcfile.c \
	rcfile.h field.c field.h hlist.c hlist.h i18n.c i18n.h \
	idxname.c idxname.h libnamazu.c libnamazu.h parser.c parser.h \
	query.c query.h re.c re.h replace.c replace.h regex.c regex.h \
	search.c search.h seed.c seed.h util.c util.h var.c var.h \
	wakati.c wakati.h 

libnmz_la_LDFLAGS = -version-info 1:0:0

add-prefix:
	echo "/* Add nmz prefix by Satoru Takabayashi */" > regex.c
	echo "/* Add nmz prefix by Satoru Takabayashi */" > regex.h
	perl get-extern-funcs.pl regex-ruby.c > extern-funcs
	perl add-prefix.pl  nmz  regex-ruby.h < extern-funcs >> regex.h
	perl add-prefix.pl  nmz  regex-ruby.c < extern-funcs |\
	perl -pe 's/\b(scan_oct|scan_hex|xmalloc|xrealloc)\b/nmz_$$1/g'\
	     >> regex.c
	rm extern-funcs
