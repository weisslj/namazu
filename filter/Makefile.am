## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.4 no-dependencies

filterdir = $(pkgdatadir)/filter

filter_DATA = bzip2.pl \
	     compress.pl \
	     gfilter.pl \
	     gzip.pl \
	     html.pl \
	     mailnews.pl \
	     man.pl \
	     mhonarc.pl \
	     msword.pl \
	     pdf.pl \
	     rfc.pl \
	     msword.pl \
	     olemsword.pl \
	     oleexcel.pl \
	     olepowerpoint.pl \
	     ichitaro456.pl \
	     tex.pl \
	     hnf.pl

EXTRA_DIST = $(filter_DATA)

ETAGS_ARGS = $(filter_DATA) --lang=perl
TAGS_DEPENDENCIES = $(filter_DATA)
