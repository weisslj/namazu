## Process this file with automake to produce Makefile.in

## This is a part of Namazu.
## Copyright (C) 1997-1999, Satoru Takabayashi. All rights reserved.

## Original of this file was made by Mr. Fumiyasu SATO. Thanks.
## Modified by Satoru Takabayashi
## 'automake'ization by Hajime BABA <baba@kusastro.kyoto-u.ac.jp>
## Thanks to [Ryuji Abe <raeva@t3.rim.or.jp>] for many contribution

AUTOMAKE_OPTIONS = 1.4 no-dependencies

bin_PROGRAMS = namazu
libexec_PROGRAMS = namazu.cgi

localedir   = $(datadir)/locale
confdir     = $(sysconfdir)/$(PACKAGE)
indexdir    = $(localstatedir)/$(PACKAGE)/index

DEFS = -DLOCALEDIR=\"$(localedir)\" \
       -DOPT_CONFDIR=\"$(confdir)\" \
       -DOPT_INDEXDIR=\"$(indexdir)\" \
       @DEFS@
INCLUDES  = -I.. -I$(srcdir)/../intl -I$(srcdir)/../lib -I$(srcdir)/../nmz
LDADD = @INTLLIBS@ $(top_builddir)/nmz/libnmz.la $(top_builddir)/lib/libnmzut.a 

common_SOURCES = em.h form.c form.h message.h \
		 namazu.c namazu.h output.c output.h rcfile.c rcfile.h \
		 result.c result.h usage.c usage.h

namazu_SOURCES     = $(common_SOURCES) namazu-cmd.c
namazu_cgi_SOURCES = $(common_SOURCES) cgi.c cgi.h namazu-cgi.c
namazu_cgi_LDFLAGS = -static

