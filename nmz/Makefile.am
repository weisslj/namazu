# Process this file with automake to produce Makefile.in
#
# $Id: Makefile.am,v 1.34 2000-10-03 03:42:16 rug Exp $
#

AUTOMAKE_OPTIONS = 1.4 no-dependencies

EXTRA_DIST = memcmp.c memmove.c memset.c strcasecmp.c strncasecmp.c \
	     strcspn.c strerror.c vsnprintf.c system.h support.h

localedir   = $(prefix)/$(DATADIRNAME)/locale
confdir     = $(sysconfdir)/$(PACKAGE)
indexdir    = $(localstatedir)/$(PACKAGE)/index

DEFS = -DLOCALEDIR=\"$(localedir)\" \
       -DOPT_CONFDIR=\"$(confdir)\" \
       -DOPT_INDEXDIR=\"$(indexdir)\" \
       -DHAVE_SUPPORT_H \
       @DEFS@
INCLUDES = -I$(srcdir) -I.. -I$(srcdir)/../lib -I$(srcdir)/../src \
	-I$(srcdir)/../intl -I$(srcdir)/../nmz

pkginclude_HEADERS = codeconv.h field.h hlist.h i18n.h idxname.h \
	libnamazu.h l10n-ja.h parser.h query.h re.h regex.h \
	search.h util.h var.h wakati.h 
lib_LTLIBRARIES = libnmz.la

libnmz_la_SOURCES = alias.c alias.h codeconv.c codeconv.h \
	field.c field.h hlist.c hlist.h i18n.c i18n.h \
	idxname.c idxname.h libnamazu.c libnamazu.h l10n-ja.c l10n-ja.h \
	parser.c parser.h query.c query.h re.c re.h replace.c replace.h \
	regex.c regex.h search.c search.h seed.c seed.h util.c util.h \
	var.c var.h wakati.c wakati.h 

libnmz_la_LIBADD = @NMZ_LIBOBJS@
libnmz_la_LDFLAGS = -version-info $(LTVERSION)

##Bug in automake: Can't use `if MAINTAINER_MODE'
@MAINTAINER_MODE_TRUE@include $(srcdir)/maintMakefile
