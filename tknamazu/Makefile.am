## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = no-dependencies
tknamazudir = @pkgdatadir@/tknamazu

bin_SCRIPTS = tknamazu
tknamazu_DATA = tknamazu.hlp tknamazu.xpm mini-tknamazu.xpm

CLEANFILES = tknamazu.tcl

EXTRA_DIST = README.tknamazu $(tknamazu_DATA)

tknamazu: tknamazu.tcl
	sed -e 's!%BINDIR%!$(bindir)!g' \
	    -e 's!%PKGDATADIR%!$(pkgdatadir)!g' \
	tknamazu.tcl > $@
	chmod +x $@
