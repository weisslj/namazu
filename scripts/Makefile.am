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

EXTRA_DIST = gcnmz.in kwnmz.in mknmz.in
CLEANFILES  = gcnmz mknmz kwnmz

localedir   = $(datadir)/locale
confdir     = $(sysconfdir)/$(PACKAGE)
filterdir   = $(pkgdatadir)/filter
indexdir    = $(localstatedir)/$(PACKAGE)/index
perllibdir  = $(pkgdatadir)/pl
templatedir = $(pkgdatadir)/template

##################################################################

gcnmz: gcnmz.in Makefile
	rm -f gcnmz
	sed -e 's!%PERL%!$(PERL)!g' \
	    -e 's!@PERLLIBDIR@!$(perllibdir)!g' $(srcdir)/gcnmz.in > gcnmz.tmp
	mv gcnmz.tmp gcnmz
	chmod +x gcnmz

kwnmz: kwnmz.in Makefile
	rm -f kwnmz
	sed -e 's!%PERL%!$(PERL)!g' \
	    -e 's!@PERLLIBDIR@!$(perllibdir)!g' $(srcdir)/kwnmz.in > kwnmz.tmp
	mv kwnmz.tmp kwnmz
	chmod +x kwnmz

mknmz: mknmz.in Makefile
	rm -f mknmz
	sed -e 's!%PERL%!$(PERL)!g' \
	    -e 's!%ADDITIONAL_INC%!$(ADDITIONAL_INC)!g' \
	    -e 's!@CONFDIR@!$(confdir)!g' \
	    -e 's!@PERLLIBDIR@!$(perllibdir)!g' \
	    -e 's!@FILTERDIR@!$(filterdir)!g' \
	    -e 's!@TEMPLATEDIR@!$(templatedir)!g' $(srcdir)/mknmz.in >mknmz.tmp 
	mv mknmz.tmp mknmz
	chmod +x mknmz

ETAGS_ARGS = $(SCRIPTS) --lang=perl
TAGS_DEPENDENCIES = $(SCRIPTS)
