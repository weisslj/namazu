## Process this file with automake to produce Makefile.in

@SET_MAKE@
AUTOMAKE_OPTIONS = 1.4 no-dependencies

pkgdatadir = @datadir@/@PACKAGE@

ext_namazu=@NAMAZU@
ext_mknmz=@MKNMZ@

bin_SCRIPTS = tknamazu
pkgdata_DATA = tknamazu.hlp.en tknamazu.hlp.ja \
	tknamazu_icon.ppm tknamazu_logo.ppm mini-tknamazu.xpm mini-tknamazu.ico \
	tknmzrc

CLEANFILES = tknamazu

EXTRA_DIST = $(pkgdata_DATA) tknamazu.spec.in tknamazu.spec

tknamazu: tknamazu.tcl
	sed -e 's!%NAMAZU%!$(ext_namazu)!g' \
	    -e 's!%MKNMZ%!$(ext_mknmz)!g' \
	    -e 's!%PKGDATADIR%!$(pkgdatadir)!g' \
	tknamazu.tcl > $@
	chmod +x $@
