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

$Id: Namazu.xs,v 1.2 1999-11-08 09:17:35 knok Exp $

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

PROTOTYPES: DISABLE

MODULE = Search::Namazu		PACKAGE = Search::Namazu

AV*
call_search_main(query)
	SV *query

	PREINIT:
		char *qstr;
		int i;
		AV *retar;
		HLIST hlist;

	CODE:
		uniq_idxnames();
		expand_idxname_aliases();
		compete_idxnames();

		qstr = SvPV(query, na);
		retar = newAV();
		hlist = search_main(qstr);
		for (i = 0; i < hlist.n; i ++) {
			HV *stash = sv_stashpv("Search::Namazu::HList", TRUE);
			SV *stashref = newRV(stash);
			get_field_data(hlist.d[i].idxid, hlist.d[i].docid, "uri", result);
			PUSHMARK(SP);
			XPUSHs(stashref);
			XPUSHs(sv_2mortal(newSViv(hlist.d[i].score)));
			XPUSHs(sv_2mortal(newSVpv(result)));
			XPUSHs(sv_2mortal(newSViv(hlist.d[i].date)));
			XPUSHs(sv_2mortal(newSViv(hlist.d[i].rank)));
			PUTBACK;
			perl_call_method("set", G_DISCARD);
			av_push(retar, stashref);
		}
		free_hlist(hlist);
		RETVAL = retar;

	OUTPUT:
		RETVAL

int
add_nmzindex(index)
	SV *index

	PREINIT:
		char *tmp;

	CODE:
		tmp = SvPV(index, na);
		RETVAL = add_index(tmp);

	OUTPUT:
		RETVAL
