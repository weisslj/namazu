## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = 1.4 no-dependencies
SUBDIRS = data

TESTS =	mknmz-1 mknmz-2 mknmz-3 mknmz-4 mknmz-5 \
	mknmz-6 mknmz-7 mknmz-8 mknmz-9 mknmz-10 \
	gcnmz-1 \
	idxdiff-1 idxdiff-2 idxdiff-3 \
	namazu-1 namazu-2 namazu-3 namazu-4 namazu-5 \
	namazu-6 namazu-7 namazu-8 namazu-9 \
	namazu-cgi-1 namazu-cgi-2 namazu-cgi-3 namazu-cgi-4

clean-local:
	rm -rf test-log tmp-data tmp.* idx[0-9]* 

EXTRA_DIST = $(TESTS) select-data
