AUTOMAKE_OPTIONS = 1.4 no-dependencies

confdir = $(sysconfdir)/$(PACKAGE)
conf_DATA = namazurc mknmzrc
EXTRA_DIST = namazurc
CLEANFILES = mknmzrc

mknmzrc: ../pl/conf.pl
	@grep -v '^package conf;$$' ../pl/conf.pl | \
	perl -pe 's/^(\$$|\@|\%)/$$1conf::/' | \
	perl -pe 's/^/\x23 / unless /^(\x23|$$)/' > $@