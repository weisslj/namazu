## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = no-dependencies

bin_SCRIPTS = tknamazu
pkgdata_DATA = tknamazu.hlp tknamazu.gif mini-tknamazu.xpm

CLEANFILES = tknamazu.tcl

EXTRA_DIST = README.tknamazu $(pkgdata_DATA)

tknamazu: tknamazu.tcl
	sed -e 's!%BINDIR%!$(bindir)!g' \
	    -e 's!%PKGDATADIR%!$(pkgdatadir)!g' \
	tknamazu.tcl > $@
	chmod +x $@
