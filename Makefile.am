## Process this file with automake to produce Makefile.in

@SET_MAKE@
EXTRA_DIST = README-ja namazu.spec.in
EXTRA_DIRS = lib
# EXTRA_DIRS = contrib doc lib misc
SUBDIRS = src

pkgdatadir = @prefix@/@PACKAGE@

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

rpm:
	# put Namazu's tar-ball to a correct directory before do this
	rpm -ba namazu.spec
