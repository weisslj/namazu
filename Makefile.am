## Process this file with automake to produce Makefile.in

@SET_MAKE@
AUTOMAKE_OPTIONS = no-dependencies

always_built_SUBDIRS = src conf

SUBDIRS = $(always_built_SUBDIRS) @FILE_MMAGIC@ @TKNAMAZU@

DIST_SUBDIRS = $(always_built_SUBDIRS) tknamazu

EXTRA_DIST = README-ja namazu.spec.in \
	     ChangeLog.old.1.gz File-MMagic

EXTRA_DIRS = doc etc lib lisp filter template

CLEANFILES = namazu.spec \
	     filter/gzip.pl filter/compress.pl filter/man.pl filter/rfc.pl \
	     filter/bzip2.pl filter/msword.pl filter/supported.pl \
	     File-MMagic/Makefile File-MMagic/pm_to_blib \
	     conf/mknmz.conf-dist lib/var.pl
CLEANDIRS  = File-MMagic/blib

pkgdatadir = @datadir@/@PACKAGE@

install-data-local:
	$(mkinstalldirs) $(pkgdatadir)/index
	for subdir in $(EXTRA_DIRS); do \
	    cp -rp $(srcdir)/$$subdir $(pkgdatadir); \
	    rm -rf $(pkgdatadir)/$$subdir/CVS; \
	    rm -rf $(pkgdatadir)/$$subdir/*/CVS; \
	    rm -f $(pkgdatadir)/$$subdir/Makefile*; \
	done
	cp ChangeLog COPYING NEWS README README-ja THANKS TODO \
		$(pkgdatadir)/etc

dist-hook:
	cp namazu.spec $(distdir)
	for subdir in $(EXTRA_DIRS); do \
	    cp -rp $$subdir $(distdir)/$$subdir; \
	    rm -f $(distdir)/$$subdir/*~; \
	    rm -f $(distdir)/$$subdir/*.{bak,orig}; \
	    rm -rf $(distdir)/$$subdir/CVS; \
	    rm -rf $(distdir)/$$subdir/*/CVS; \
	done
	rm -f $(distdir)/$(CLEANFILES)
	rm -rf $(distdir)/$(CLEANDIRS)

