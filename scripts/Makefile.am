## Process this file with automake to produce Makefile.in

## This is a part of Namazu.
## Copyright (C) 1997-1999, Satoru Takabayashi. All rights reserved.

## Original of this file was made by Mr. Fumiyasu SATO. Thanks.
## Modified by Satoru Takabayashi
## 'automake'ization by Hajime BABA <baba@kusastro.kyoto-u.ac.jp>
## Thanks to [Ryuji Abe <raeva@t3.rim.or.jp>] for many contribution

AUTOMAKE_OPTIONS = 1.4 no-dependencies

bin_SCRIPTS    = gcnmz mailutime mknmz vfnmz kwnmz nmzgrep bnamazu
noinst_SCRIPTS = gtnmz

CLEANFILES  = gcnmz mknmz kwnmz

localedir   = $(datadir)/locale
confdir     = $(sysconfdir)/$(PACKAGE)
filterdir   = $(pkgdatadir)/filter
indexdir    = $(localstatedir)/$(PACKAGE)/index
perllibdir  = $(pkgdatadir)/pl
templatedir = $(pkgdatadir)/template

##################################################################

gcnmz: gcnmz.pl
	rm -f $@
	sed -e 's!%LIBDIR%!$(perllibdir)!g' $< > $@.tmp
	mv $@.tmp $@
	chmod +x $@

kwnmz: kwnmz.pl
	rm -f $@
	sed -e 's!%LIBDIR%!$(perllibdir)!g' $< > $@.tmp
	mv $@.tmp $@
	chmod +x $@

mknmz: mknmz.pl
	rm -f $@
	sed -e 's!%CONFDIR%!$(confdir)!g' \
	    -e 's!%LIBDIR%!$(perllibdir)!g' \
	    -e 's!%FILTERDIR%!$(filterdir)!g' \
	    -e 's!%TEMPLATEDIR%!$(templatedir)!g' $< >$@.tmp 
	mv $@.tmp $@
	chmod +x $@

ETAGS_ARGS = $(SCRIPTS) --lang=perl
TAGS_DEPENDENCIES = $(SCRIPTS)
