## Process this file with automake to produce Makefile.in

@SET_MAKE@
AUTOMAKE_OPTIONS = no-dependencies

pkgdatadir = @datadir@/@PACKAGE@

bin_SCRIPTS = tknamazu
pkgdata_DATA = tknamazu.hlp tknamazu.ppm mini-tknamazu.xpm

CLEANFILES = tknamazu

EXTRA_DIST = $(pkgdata_DATA)

tknamazu: tknamazu.tcl
	sed -e 's!%BINDIR%!$(bindir)!g' \
	    -e 's!%PKGDATADIR%!$(pkgdatadir)!g' \
	tknamazu.tcl > $@
	chmod +x $@
