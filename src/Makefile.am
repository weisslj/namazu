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

##################################################################

DEFS = -DLOCALEDIR=\"$(localedir)\" \
       -DOPT_CONFDIR=\"$(confdir)\" \
       -DOPT_INDEXDIR=\"$(indexdir)\" \
       @DEFS@
INCLUDES  = -I.. -I../intl -I../lib -I../nmz
LDADD = @INTLLIBS@ ../lib/libnmzut.a $(top_srcdir)/nmz/libnmz.la

namazu_SOURCES = cgi.c form.c \
		 i18n.c namazu.c output.c \
		 result.c \
		 usage.c \
		 cgi.h em.h form.h \
		 i18n.h message.h namazu.h output.h \
		 result.h \
		 usage.h

SUFFIXES = .pl
.pl:
	rm -f $@
	sed -e 's!%CONFDIR%!$(confdir)!g' \
	    -e 's!%LIBDIR%!$(perllibdir)!g' \
	    -e 's!%FILTERDIR%!$(filterdir)!g' \
	    -e 's!%TEMPLATEDIR%!$(templatedir)!g' \
	$< > $@
	chmod +x $@
