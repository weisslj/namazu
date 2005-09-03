## Process this file with automake to produce Makefile.in

## This is a part of Namazu.
## Copyright (C) 1997-1999, Satoru Takabayashi. All rights reserved.

## Original of this file was made by Mr. Fumiyasu SATO. Thanks.
## Modified by Satoru Takabayashi
## 'automake'ization by Hajime BABA <baba@kusastro.kyoto-u.ac.jp>
## Thanks to [Ryuji Abe <raeva@t3.rim.or.jp>] for many contribution

AUTOMAKE_OPTIONS = 1.4 no-dependencies

wakatidictdir = $(pkgdatadir)/wakatidict
wakatidict_DATA = kakasidict.dir kakasidict.pag
EXTRA_DIST = kakasidict
CLEANFILES  = kakasidict.pag kakasidict.dir

kakasidict.dir: kakasidict
	@dicname=`basename $@ .dir`; \
	rm -f $$dicname.dir; \
	rm -f $$dicname.pag; \
	../scripts/load_dict -i $$dicname 

