## Process this file with automake to produce Makefile.in

## This is a part of Namazu.
## Copyright (C) 1997-1999, Satoru Takabayashi. All rights reserved.

## Original of this file was made by Mr. Fumiyasu SATO. Thanks.
## Modified by Satoru Takabayashi
## 'automake'ization by Hajime BABA <baba@kusastro.kyoto-u.ac.jp>
## Thanks to [Ryuji Abe <raeva@t3.rim.or.jp>] for many contribution

AUTOMAKE_OPTIONS = 1.4 no-dependencies

bin_SCRIPTS    = gcnmz mailutime mknmz vfnmz kwnmz nmzgrep
noinst_SCRIPTS = gtnmz

CLEANFILES  = gcnmz mknmz kwnmz

localedir   = $(datadir)/locale
confdir     = $(sysconfdir)/$(PACKAGE)
filterdir   = $(pkgdatadir)/filter
indexdir    = $(localstatedir)/$(PACKAGE)/index
perllibdir  = $(pkgdatadir)/pl
templatedir = $(pkgdatadir)/template

##################################################################

SUFFIXES = .pl
.pl:
	rm -f $@
	sed -e 's!%CONFDIR%!$(confdir)!g' \
	    -e 's!%LIBDIR%!$(perllibdir)!g' \
	    -e 's!%FILTERDIR%!$(filterdir)!g' \
	    -e 's!%TEMPLATEDIR%!$(templatedir)!g' \
	$< > $@
	chmod +x $@

#tags: TAGS
#TAGS:  $(SCRIPTS) $(TAGS_DEPENDENCIES) $(LISP)
#	tags=; \
#	here=`pwd`; \
#	list='$(SCRIPTS)'; \
#	unique=`for i in $$list; do echo $$i; done | \
#	  awk '    { files[$$0] = 1; } \
#	       END { for (i in files) print i; }'`; \
#	test -z "$(ETAGS_ARGS)$$unique$(LISP)$$tags" \
#	  || (cd $(srcdir) && etags $(ETAGS_ARGS) $$tags  $$unique $(LISP) -o $$here/TAGS)
