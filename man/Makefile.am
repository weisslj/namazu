# Process this file with automake to produce Makefile.in

man_MANS = namazu.1 mknmz.1
EXTRA_DIST = help2man $(man_MANS)
HELP2MAN = $(PERL) -w -- $(srcdir)/help2man

update-man:
	$(HELP2MAN) --no-info $(top_builddir)/src/namazu > namazu.1
	$(HELP2MAN) --no-info $(top_builddir)/scripts/mknmz > mknmz.1

