## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = 1.4 no-dependencies
SUBDIRS = data

TESTS =	mknmz-1 mknmz-2 mknmz-3 mknmz-4 mknmz-5 \
	gcnmz-1 \
	idxdiff-1 idxdiff-2 idxdiff-3 \
	namazu-1 namazu-2 namazu-3 namazu-4 \
	namazu-cgi-1 namazu-cgi-2

clean-local:
	rm -rf tmp-data idx* test-log

EXTRA_DIST = $(TESTS)
