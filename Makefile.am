## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.4 no-dependencies

if BUILD_TKNAMAZU
BUILT_TKNAMAZU = tknamazu
DIST_TKNAMAZU =
else
BUILT_TKNAMAZU =
DIST_TKNAMAZU = tknamazu
endif

SUBDIRS      = conf doc etc filter intl po nmz lib pl src lisp man scripts \
	  template $(BUILT_TKNAMAZU)

DIST_SUBDIRS = conf doc etc filter intl po nmz lib pl src lisp man scripts \
	  template $(BUILT_TKNAMAZU)


bin_SCRIPTS = nmz-config

etcdir     = $(pkgdatadir)/etc
etc_DATA   = README-ja INSTALL-ja namazu.spec.in namazu.spec \
	     CREDITS ChangeLog.1 

EXTRA_DIST = $(etc_DATA) maintMakefile


EXTRA_DIRS = $(DIST_TKNAMAZU) File-MMagic

install-data-local:
	$(mkinstalldirs) $(localstatedir)/$(PACKAGE)/index

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

##Bug in automake: Can't use `if MAINTAINER_MODE'
@MAINTAINER_MODE_TRUE@include $(srcdir)/maintMakefile
