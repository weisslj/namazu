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

$Id: Namazu.xs,v 1.10 2000-01-24 05:35:23 knok Exp $

*/

#ifdef __cplusplus
extern "C" {
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "libnamazu.h"
#include "codeconv.h"
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
		NmzResult hlist;
		char result[BUFSIZE];

		nmz_uniq_idxnames();
		nmz_expand_idxname_aliases();
		nmz_complete_idxnames();

		qstr = SvPV(query, na);
		strcpy(cqstr, qstr);
		nmz_codeconv_query(cqstr);
		retar = newAV();
		hlist = nmz_search(cqstr);
		for (i = 0; i < hlist.num; i ++) {
			SV *ohlist = perl_eval_pv("new Search::Namazu::Result", TRUE);
			SV *tmp;
			nmz_get_field_data(hlist.data[i].idxid, hlist.data[i].docid, "uri", result);
			PUSHMARK(SP);
			XPUSHs(ohlist);
			XPUSHs(sv_2mortal(newSViv(hlist.data[i].score)));
			XPUSHs(sv_2mortal(newSVpv(result, strlen(result))));
			XPUSHs(sv_2mortal(newSViv(hlist.data[i].date)));
			XPUSHs(sv_2mortal(newSViv(hlist.data[i].rank)));
			PUTBACK;
			perl_call_method("set", G_DISCARD);
			POPs;
			POPs;
			POPs;
			POPs;
			POPs;
			av_push(retar, ohlist);
		}
		nmz_free_hlist(hlist);
		EXTEND(SP, hlist.num);
		for (i = 0; i < hlist.num; i ++) {
			XPUSHs(av_pop(retar));
		}
		nmz_free_internal();

int
nmz_addindex(index)
	SV *index

	PREINIT:
		char *tmp;

	CODE:
		tmp = SvPV(index, na);
		RETVAL = nmz_add_index(tmp);

	OUTPUT:
		RETVAL

void
nmz_sortbydate()
	CODE:
		nmz_set_sortmethod(SORT_BY_SCORE);

void
nmz_sortbyscore()
	CODE:
		nmz_set_sortmethod(SORT_BY_DATE);

void
nmz_sortbyfield()
	CODE:
		nmz_set_sortmethod(SORT_BY_FIELD);

void
nmz_descendingsort()
	CODE:
		nmz_set_sortorder(ASCENDING);

void
nmz_ascendingsort()
	CODE:
		nmz_set_sortorder(DESCENDING);

int
nmz_setlang(lang)
	SV *lang

	PREINIT:
		char *tmp;

	CODE:
		tmp = SvPV(lang, na);
		RETVAL = nmz_set_lang(tmp);

	OUTPUT:
		RETVAL
