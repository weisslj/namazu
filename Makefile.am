## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.4 no-dependencies

SUBDIRS = intl po conf filter nmz lib pl src lisp man scripts \
	  @FILE_MMAGIC@ @TKNAMAZU@

DIST_SUBDIRS = $(SUBDIRS) tknamazu

bin_SCRIPTS = nmz-config

EXTRA_DIST = README-ja INSTALL-ja namazu.spec.in namazu.spec pxgettext \
	     ChangeLog.1 maintMakefile File-MMagic 

EXTRA_DIRS = doc etc template

DISTCLEANFILES = File-MMagic/Makefile File-MMagic/pm_to_blib \
		 conf/mknmz.conf-dist lib/var.pl lib/conf.pl

DISTCLEANDIRS  = File-MMagic/blib File-MMagic/CVS

pkgdatadir = @datadir@/@PACKAGE@

install-data-local:
	$(mkinstalldirs) $(localstatedir)/$(PACKAGE)/index
	for subdir in $(EXTRA_DIRS); do \
	    cp -rp $(srcdir)/$$subdir $(pkgdatadir); \
	    rm -rf $(pkgdatadir)/$$subdir/CVS; \
	    rm -rf $(pkgdatadir)/$$subdir/*/CVS; \
	    rm -f $(pkgdatadir)/$$subdir/Makefile*; \
	done
	cp ChangeLog COPYING NEWS README README-ja THANKS TODO \
		$(pkgdatadir)/etc
	rm -f $(pkgdatadir)/template/*.in

dist-hook:
	for subdir in $(EXTRA_DIRS); do \
	    cp -rp $$subdir $(distdir)/$$subdir; \
	    rm -f $(distdir)/$$subdir/*~; \
	    rm -f $(distdir)/$$subdir/*.{bak,orig}; \
	    rm -rf $(distdir)/$$subdir/CVS; \
	    rm -rf $(distdir)/$$subdir/*/CVS; \
	done

	for file in $(DISTCLEANFILES); do \
	    rm -f $(distdir)/$$file; \
	done

	for dir in $(DISTCLEANDIRS); do \
	    rm -rf $(distdir)/$$dir; \
	done

##Bug in automake: Can't use `if MAINTAINER_MODE'
@MAINTAINER_MODE_TRUE@include $(srcdir)/maintMakefile
