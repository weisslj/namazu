## Process this file with automake to produce Makefile.in

## This is a part of Namazu.
## Copyright (C) 1997-1999, Satoru Takabayashi. All rights reserved.

## Original of this file was made by Mr. Fumiyasu SATO. Thanks.
## Modified by Satoru Takabayashi
## 'automake'ization by Hajime BABA <baba@kusastro.kyoto-u.ac.jp>
## Thanks to [Ryuji Abe <raeva@t3.rim.or.jp>] for many contribution

AUTOMAKE_OPTIONS = 1.4 no-dependencies

bin_PROGRAMS = namazu
bin_SCRIPTS  = gcnmz mailutime mknmz gtnmz vfnmz kwnmz

CLEANFILES  = gcnmz mknmz kwnmz

localedir   = $(datadir)/locale
confdir     = $(sysconfdir)/$(PACKAGE)
filterdir   = $(pkgdatadir)/filter
indexdir    = $(localstatedir)/$(PACKAGE)/index
perllibdir  = $(pkgdatadir)/pl
templatedir = $(pkgdatadir)/template

DEFS = -DLOCALEDIR=\"$(localedir)\" \
       -DOPT_CONFDIR=\"$(confdir)\" \
       -DOPT_INDEXDIR=\"$(indexdir)\" \
       @DEFS@
INCLUDES  = -I.. -I../intl -I../lib -I../nmz
LDADD = @INTLLIBS@ ../lib/libnmzut.a $(top_srcdir)/nmz/libnmz.la

namazu_SOURCES = cgi.c codeconv.c conf.c field.c form.c \
		 hlist.c i18n.c namazu.c output.c \
		 parser.c re.c result.c search.c seed.c util.c \
		 usage.c var.c wakati.c idxname.c \
		 cgi.h codeconv.h conf.h critical.h em.h field.h form.h \
		 hlist.h i18n.h magic.h message.h namazu.h output.h \
		 parser.h re.h result.h search.h \
		 usage.h  util.h wakati.h var.h idxname.h

SUFFIXES = .pl
.pl:
	rm -f $@
	sed -e 's!%CONFDIR%!$(confdir)!g' \
	    -e 's!%LIBDIR%!$(perllibdir)!g' \
	    -e 's!%FILTERDIR%!$(filterdir)!g' \
	    -e 's!%TEMPLATEDIR%!$(templatedir)!g' \
	$< > $@
	chmod +x $@
