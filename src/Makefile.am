## Process this file with automake to produce Makefile.in

## This is a part of Namazu.
## Copyright (C) 1997-1999, Satoru Takabayashi. All rights reserved.

## Original of this file was made by Mr. Fumiyasu SATO. Thanks.
## Modified by Satoru Takabayashi
## 'automake'ization by Hajime BABA <baba@kusastro.kyoto-u.ac.jp>
## Thanks to [Ryuji Abe <raeva@t3.rim.or.jp>] for many contribution

@SET_MAKE@
LIBS = -lm @LEXLIB@

bin_PROGRAMS = namazu
bin_SCRIPTS = gcnmz mailutime mknmz gtnmz

CLEANFILES = mknmz.pl $(bin_SCRIPTS)

pkgdatadir  = @datadir@/@PACKAGE@
confdir     = $(pkgdatadir)/conf
libdir      = $(pkgdatadir)/lib
filterdir   = $(pkgdatadir)/filter
templatedir = $(pkgdatadir)/template

namazu_SOURCES = cgi.c codeconv.c conf.c field.c form.c getopt.c \
		 getopt1.c hlist.c message.c namazu.c output.c parser.c re.c \
		 result.l regex.c search.c seed.c util.c usage.c var.c wakati.c
noinst_HEADERS = namazu.h regex.h util.h getopt.h field.h codeconv.h \
		 form.h usage.h hlist.h re.h output.h conf.h em.h \
		 magic.h critical.h

##################################################################

mknmz: mknmz.pl
	rm -f $@
	sed -e 's!%CONFDIR%!$(confdir)!g' \
	    -e 's!%LIBDIR%!$(libdir)!g' \
	    -e 's!%FILTERDIR%!$(filterdir)!g' \
	    -e 's!%TEMPLATEDIR%!$(templatedir)!g' \
	mknmz.pl > $@
	chmod +x $@

