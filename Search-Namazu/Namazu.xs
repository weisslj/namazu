/*

Namazu.xs

# Copyright (C) 1999 NOKUBI Takatsugu All rights reserved.
# This is free software with ABSOLUTELY NO WARRANTY.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA

$Id: Namazu.xs,v 1.8 1999-11-16 08:42:35 knok Exp $

*/

#ifdef __cplusplus
extern "C" {
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "libnamazu.h"
#include "codeconv.h"
#include "conf.h"
#include "field.h"
#include "hlist.h"
#include "idxname.h"
#include "parser.h"
#include "re.h"
#include "search.h"
#include "util.h"
#include "wakati.h"
#ifdef __cplusplus
}
#endif


MODULE = Search::Namazu		PACKAGE = Search::Namazu

PROTOTYPES: DISABLE

void
call_search_main(query)
	SV *query

	PPCODE:
		char *qstr;
		char cqstr[BUFSIZE * 2];
		int i;
		AV *retar;
		HLIST hlist;
		char result[BUFSIZE];

		uniq_idxnames();
		expand_idxname_aliases();
		complete_idxnames();

		qstr = SvPV(query, na);
		strcpy(cqstr, qstr);
		codeconv_query(cqstr);
		retar = newAV();
		hlist = search_main(cqstr);
		for (i = 0; i < hlist.n; i ++) {
			SV *ohlist = perl_eval_pv("new Search::Namazu::HList", TRUE);
			SV *tmp;
			get_field_data(hlist.d[i].idxid, hlist.d[i].docid, "uri", result);
			PUSHMARK(SP);
			XPUSHs(ohlist);
			XPUSHs(sv_2mortal(newSViv(hlist.d[i].score)));
			XPUSHs(sv_2mortal(newSVpv(result, strlen(result))));
			XPUSHs(sv_2mortal(newSViv(hlist.d[i].date)));
			XPUSHs(sv_2mortal(newSViv(hlist.d[i].rank)));
			PUTBACK;
			perl_call_method("set", G_DISCARD);
			POPs;
			POPs;
			POPs;
			POPs;
			POPs;
			av_push(retar, ohlist);
		}
		free_hlist(hlist);
		EXTEND(SP, hlist.n);
		for (i = 0; i < hlist.n; i ++) {
			XPUSHs(av_pop(retar));
		}
		free_idxnames();
		free_aliases();
		free_replaces();

int
nmz_addindex(index)
	SV *index

	PREINIT:
		char *tmp;

	CODE:
		tmp = SvPV(index, na);
		RETVAL = add_index(tmp);

	OUTPUT:
		RETVAL

void
nmz_sortbydate()
	CODE:
		set_sortbydate();

void
nmz_sortbyscore()
	CODE:
		set_sortbyscore();

void
nmz_sortbyfield()
	CODE:
		set_sortbyfield();

void
nmz_descendingsort()
	CODE:
		set_sort_descending();

void
nmz_ascendingsort()
	CODE:
		set_sort_ascending();

int
nmz_setconfig(conf)
	SV *conf

	PREINIT:
		char *tmp;

	CODE:
		tmp = SvPV(conf, na);
		RETVAL = load_conf(tmp);

	OUTPUT:
		RETVAL

int
nmz_setlang(lang)
	SV *lang

	PREINIT:
		char *tmp;

	CODE:
		tmp = SvPV(lang, na);
		RETVAL = set_lang(tmp);

	OUTPUT:
		RETVAL
