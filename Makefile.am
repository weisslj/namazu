## Process this file with automake to produce Makefile.in

@SET_MAKE@
EXTRA_DIST = README-ja namazu.spec.in
EXTRA_DIRS = lib filter template
# EXTRA_DIRS = contrib doc lib misc
SUBDIRS = src

CLEANFILES = lib/conf.pl namazu.spec \
	     filter/gzip.pl filter/compress.pl filter/man.pl filter/rfc.pl \
	     filter/bzip2.pl filter/msword.pl 

pkgdatadir = @datadir@/@PACKAGE@/@VERSION@

install-data-local:
	$(mkinstalldirs) $(pkgdatadir)/index
	for subdir in $(EXTRA_DIRS); do \
	  cp -rp $(srcdir)/$$subdir $(pkgdatadir); \
	done

dist-hook:
	for subdir in $(EXTRA_DIRS); do \
	  cp -rp $$subdir $(distdir)/$$subdir; \
	  rm -f $(distdir)/$$subdir/*~; \
	  rm -f $(distdir)/$$subdir/*.{bak,orig}; \
	  rm -rf $(distdir)/$$subdir/CVS; \
	done
	rm -f $(distdir)/lib/conf.pl
	rm -f $(distdir)/filter/gzip.pl
	rm -f $(distdir)/filter/compress.pl
	rm -f $(distdir)/filter/man.pl
	rm -f $(distdir)/filter/rfc.pl
	rm -f $(distdir)/filter/bzip2.pl
	rm -f $(distdir)/filter/pdf.pl
	rm -f $(distdir)/filter/msword.pl

rpm:
	# put Namazu's tar-ball to a correct directory before do this
	rpm -ba namazu.spec
