## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.4 no-dependencies

filterdir = $(pkgdatadir)/filter/win32

filter_DATA = olemsword.pl \
	     oleexcel.pl \
	     olepowerpoint.pl \
	     ichitaro456.pl \
	     oletaro4.pl \
	     oletaro5.pl \
	     oletaro6.pl \
	     oletaro7.pl \
	     oletaro8.pl \
	     olertf.pl

EXTRA_DIST = $(filter_DATA)

ETAGS_ARGS = $(filter_DATA) --lang=perl
TAGS_DEPENDENCIES = $(filter_DATA)
