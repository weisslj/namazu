# Process this file with automake to produce Makefile.in

man_MANS = namazu.1 mknmz.1
man_aux = $(man_MANS:.1=.x)

EXTRA_DIST = help2man $(man_MANS) $(man_aux)
MAINTAINERCLEANFILES = $(man_MANS)
HELP2MAN = $(PERL) -w -- $(srcdir)/help2man

namazu.1:
	@progname=`basename $@ .1`; \
	executable=$(top_builddir)/src/$$progname; \
	if test -f $$executable; then \
	 rm -f t-$$progname.1; \
	 $(HELP2MAN) --include=$$progname.x --no-info \
	 $$executable > t-$$progname.1 || exit 1; \
	 mv t-$$progname.1 $$progname.1; \
	else \
	 echo "WARNING: The man page $@ cannot be updated yet."; \
	fi

mknmz.1:
	@progname=`basename $@ .1`; \
	executable=$(top_builddir)/scripts/$$progname; \
	if test -f $$executable; then \
	 rm -f t-$$progname.1; \
	 $(HELP2MAN) --include=$$progname.x --no-info \
	 $$executable > t-$$progname.1 || exit 1; \
	 mv t-$$progname.1 $$progname.1; \
	else \
	 echo "WARNING: The man page $@ cannot be updated yet."; \
	fi
