## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.4 no-dependencies

filterdir = $(pkgdatadir)/filter/win32

filter_DATA = msword.pl \
	     excel.pl \
	     powerpoint.pl \
	     ichitaro456.pl

EXTRA_DIST = $(filter_DATA)

ETAGS_ARGS = $(filter_DATA) --lang=perl
TAGS_DEPENDENCIES = $(filter_DATA)
