## Process this file with automake to produce Makefile.in

## This is a part of Namazu.
## Copyright (C) 1997-1999, Satoru Takabayashi. All rights reserved.

## Original of this file was made by Mr. Fumiyasu SATO. Thanks.
## Modified by Satoru Takabayashi
## 'automake'ization by Hajime BABA <baba@kusastro.kyoto-u.ac.jp>
## Thanks to [Ryuji Abe <raeva@t3.rim.or.jp>] for many contribution

@SET_MAKE@

##################################################################
## Directories

exec_prefix =	@prefix@/@PACKAGE@

bin_PROGRAMS = namazu
bin_SCRIPTS = clnmz gcnmz mailutime mknmz rvnmz vfnmz wdnmz gtnmz

namazu_SOURCES = cgi.c codeconv.c conf.c form.c hlist.c messages.c \
	namazu.c output.c parser.c re_match.c regex.c search.c \
	seed.c util.c values.c wakati.c
noinst_HEADERS = namazu.h regex.h util.h

##################################################################

mknmz: mknmz.pl
	rm -f $@
	sed -e 's!%LIBDIR%!$(libdir)!g' \
	mknmz.pl > $@
	chmod +x $@
