## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.4 no-dependencies

if BUILD_TKNAMAZU
BUILT_TKNAMAZU = tknamazu
DIST_TKNAMAZU =
else
BUILT_TKNAMAZU =
DIST_TKNAMAZU = tknamazu
endif

SUBDIRS = intl po conf filter nmz lib pl src lisp man scripts \
	  template $(BUILT_TKNAMAZU)

DIST_SUBDIRS = intl po conf filter nmz lib pl src lisp man scripts \
	  template $(BUILT_TKNAMAZU)


bin_SCRIPTS = nmz-config

EXTRA_DIST = README-ja INSTALL-ja namazu.spec.in namazu.spec \
	     CREDITS ChangeLog.1 maintMakefile

COMMON_DIRS = doc etc
EXTRA_DIRS = $(COMMON_DIRS) $(DIST_TKNAMAZU) File-MMagic

install-data-local:
	$(mkinstalldirs) $(localstatedir)/$(PACKAGE)/index
	for subdir in $(COMMON_DIRS); do \
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
	    rm -f $(distdir)/$$subdir/.cvsignore; \
	    rm -f $(distdir)/$$subdir/autogen.sh; \
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
