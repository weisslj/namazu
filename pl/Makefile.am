## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.4 no-dependencies

localedir   = $(datadir)/locale
perllibdir = $(pkgdatadir)/pl

perllib_DATA = \
	codeconv.pl \
	conf.pl \
	gettext.pl \
	nmzidx.pl \
	seed.pl \
	usage.pl \
	util.pl \
	var.pl \
	wakati.pl

EXTRA_DIST = \
	codeconv.pl \
	conf.pl \
	gettext.pl.in \
	nmzidx.pl \
	seed.pl \
	usage.pl \
	util.pl \
	var.pl \
	wakati.pl

CLEANFILES: gettext.pl

gettext.pl: gettext.pl.in Makefile
	rm -f gettext.pl
	sed -e 's!@LOCALEDIR@!$(localedir)!g' \
	gettext.pl.in > gettext.pl

ETAGS_ARGS = $(perllib_DATA) --lang=perl
TAGS_DEPENDENCIES = $(perllib_DATA)
