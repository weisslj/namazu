## Process this file with automake to produce Makefile.in

@SET_MAKE@
EXTRA_DIST = README-ja namazu.spec.in
EXTRA_DIRS = conf doc etc lib lisp filter template tknamazu
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
	  rm -rf $(pkgdatadir)/$$subdir/CVS; \
	  rm -rf $(pkgdatadir)/$$subdir/*/CVS; \
	done
	cp ChangeLog COPYING NEWS README README-ja THANKS TODO \
		$(pkgdatadir)/etc

dist-hook:
	for subdir in $(EXTRA_DIRS); do \
	  cp -rp $$subdir $(distdir)/$$subdir; \
	  rm -f $(distdir)/$$subdir/*~; \
	  rm -f $(distdir)/$$subdir/*.{bak,orig}; \
	  rm -rf $(distdir)/$$subdir/CVS; \
	done
	rm -f $(distdir)/$(CLEANFILES)

rpm:
	# put Namazu's tar-ball to a correct directory before do this
	rpm -ba namazu.spec
