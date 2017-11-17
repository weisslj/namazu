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

filter_DATA = apachecache.pl \
	      applefile.pl \
	      bzip2.pl \
	      compress.pl \
	      deb.pl \
	      dvi.pl \
	      epub.pl \
	      excel.pl \
	      gnumeric.pl \
	      gzip.pl \
	      hdml.pl \
	      hnf.pl \
	      html.pl \
	      koffice.pl \
	      macbinary.pl \
	      mailnews.pl \
	      man.pl \
	      markdown.pl \
	      mhonarc.pl \
	      mp3.pl \
	      msofficexml.pl \
	      msword.pl \
	      ooo.pl \
	      pdf.pl \
	      pipermail.pl \
	      postscript.pl \
	      powerpoint.pl \
	      rfc.pl \
	      rpm.pl \
	      rtf.pl \
	      taro.pl \
	      taro56.pl \
	      taro7_10.pl \
	      tex.pl \
	      visio.pl \
	      xps.pl \
	      zip.pl

EXTRA_DIST = $(filter_DATA)

ETAGS_ARGS = $(filter_DATA) --lang=perl
TAGS_DEPENDENCIES = $(filter_DATA)

# To remove Windows specific files installed by Namazu 2.0 and/or 2.0.1
# and old gfilter.pl (Namazu 2.0.7 or earlier).
install-data-local:
	rm -f $(filterdir)/ole*.pl
	rm -f $(filterdir)/gfilter.pl
