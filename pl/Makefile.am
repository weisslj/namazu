## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.4 no-dependencies

localedir   = $(prefix)/$(DATADIRNAME)/locale
perllibdir = $(pkgdatadir)/pl

perllib_DATA = \
	codeconv.pl \
	conf.pl \
	gettext.pl \
	htmlsplit.pl \
	nmzidx.pl \
	seed.pl \
	usage.pl \
	util.pl \
	var.pl \
	wakati.pl

# Slightly different from perllib_DATA because of *.in files.
EXTRA_DIST = $\
	codeconv.pl \
	conf.pl \
	gettext.pl.in \
	htmlsplit.pl \
	nmzidx.pl \
	seed.pl \
	usage.pl \
	util.pl \
	var.pl \
	wakati.pl

CLEANFILES = gettext.pl

gettext.pl: gettext.pl.in Makefile
	rm -f gettext.pl
	sed -e 's!@LOCALEDIR@!$(localedir)!g' \
	$(srcdir)/gettext.pl.in > gettext.pl

ETAGS_ARGS = $(perllib_DATA) --lang=perl
TAGS_DEPENDENCIES = $(perllib_DATA)
