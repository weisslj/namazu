## Process this file with automake to produce Makefile.in

@SET_MAKE@
AUTOMAKE_OPTIONS = 1.4 no-dependencies

pkgdatadir = @datadir@/@PACKAGE@

ext_namazu=@NAMAZU@
ext_mknmz=@MKNMZ@

bin_SCRIPTS = tknamazu
pkgdata_DATA = tknamazu.hlp.en tknamazu.hlp.ja \
	tknamazu_icon.ppm tknamazu_logo.ppm mini-tknamazu.xpm mini-tknamazu.ico \
	tknamazurc

CLEANFILES = tknamazu

EXTRA_DIST = $(pkgdata_DATA) tknamazu.spec.in tknamazu.spec

tknamazu: tknamazu.tcl
	sed -e 's!%PKGDATADIR%!$(pkgdatadir)!g' \
	tknamazu.tcl > $@
	chmod +x $@
