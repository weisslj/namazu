## Process this file with automake to produce Makefile.in

## This is a part of Namazu.
## Copyright (C) 1997-1999, Satoru Takabayashi. All rights reserved.

## Original of this file was made by Mr. Fumiyasu SATO. Thanks.
## Modified by Satoru Takabayashi
## 'automake'ization by Hajime BABA <baba@kusastro.kyoto-u.ac.jp>
## Thanks to [Ryuji Abe <raeva@t3.rim.or.jp>] for many contribution

AUTOMAKE_OPTIONS = 1.4 no-dependencies

bin_PROGRAMS = namazu
noinst_PROGRAMS = namazu.cgi

localedir   = $(datadir)/locale
confdir     = $(sysconfdir)/$(PACKAGE)
indexdir    = $(localstatedir)/$(PACKAGE)/index

##################################################################

DEFS = -DLOCALEDIR=\"$(localedir)\" \
       -DOPT_CONFDIR=\"$(confdir)\" \
       -DOPT_INDEXDIR=\"$(indexdir)\" \
       @DEFS@
INCLUDES  = -I.. -I../intl -I../lib -I../nmz
LDADD = @INTLLIBS@ $(top_srcdir)/nmz/libnmz.la  ../lib/libnmzut.a 

namazu_SOURCES = cgi.c cgi.h em.h form.c form.h message.h \
		 namazu.c namazu.h output.c output.h rcfile.c rcfile.h \
		 result.c result.h usage.c usage.h

namazu_cgi_SOURCES = $(namazu_SOURCES)
namazu_cgi_LDFLAGS = -static

