# Process this file with automake to produce Makefile.in

man_MANS = namazu.1 mknmz.1
EXTRA_DIST = $(man_MANS)

update-man:
	LANG=C help2man --no-info ../src/namazu > namazu.1
	LANG=C help2man --no-info ../src/mknmz > mknmz.1

