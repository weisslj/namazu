## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.4 no-dependencies

if WIN32
win32 = win32
else
win32 = 
endif

SUBDIRS = $(win32)
DIST_SUBDIRS = win32

filterdir = $(pkgdatadir)/filter

filter_DATA = bzip2.pl \
	     compress.pl \
	     excel.pl \
	     gfilter.pl \
	     gzip.pl \
	     html.pl \
	     mailnews.pl \
	     man.pl \
	     mhonarc.pl \
	     msword.pl \
	     pdf.pl \
	     rfc.pl \
	     tex.pl \
	     hnf.pl

EXTRA_DIST = $(filter_DATA)

ETAGS_ARGS = $(filter_DATA) --lang=perl
TAGS_DEPENDENCIES = $(filter_DATA)

# To remove Windows specific files installed by Namazu 2.0 or/and 2.0.1
install-data-local:
	rm -f $(filterdir)/ole*.pl
	rm -f $(filterdir)/ichitaro.pl
