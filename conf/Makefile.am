AUTOMAKE_OPTIONS = 1.4 no-dependencies

confdir = $(pkgdatadir)/conf
conf_DATA = namazu.conf-dist mknmz.conf-dist
EXTRA_DIST = namazu.conf-dist
CLEANFILES = mknmz.conf-dist

mknmz.conf-dist: ../pl/conf.pl
	@grep -v '^package conf;$$' ../pl/conf.pl | \
	perl -pe 's/^(\$$|\@|\%)/$$1conf::/' > $@