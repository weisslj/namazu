## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.4 no-dependencies

localedir   = $(datadir)/locale
perllibdir = $(pkgdatadir)/pl

perllib_DATA = \
	codeconv.pl \
	conf.pl \
	find.pl \
	nmzidx.pl \
	seed.pl \
	usage.pl \
	util.pl \
	var.pl \
	wakati.pl

EXTRA_DIST = \
	codeconv.pl \
	conf.pl.in \
	find.pl \
	nmzidx.pl \
	seed.pl \
	usage.pl \
	util.pl \
	var.pl.in \
	wakati.pl

gettext.pl: gettext.pl.in
	rm -f @
	sed -e 's!%LOCALEDIR%!$(localedir)!g' \
	$gettext.pl.in > $@
	chmod +x $@

ETAGS_ARGS = $(perllib_DATA) --lang=perl
