AUTOMAKE_OPTIONS = 1.4 no-dependencies

confdir = $(sysconfdir)/$(PACKAGE)
conf_DATA  = namazurc-sample mknmzrc-sample
EXTRA_DIST = namazurc-sample
CLEANFILES = mknmzrc-sample

mknmzrc-sample: ../pl/conf.pl
	perl -pe 's/^/\x23 / unless /^(\x23|package|$$)/' ../pl/conf.pl > $@
