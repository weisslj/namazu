EXTRA_DIST = namazu.conf-dist
all: mknmz.conf-dist
mknmz.conf-dist: ../lib/conf.pl
	@grep -v '^package conf;$$' ../lib/conf.pl | \
	perl -pe 's/^(\$$|\@|\%)/$$1conf::/' > $@