/*
 * result.l -
 * -*- C -*-
 * $Id: result.l,v 1.20 1999-09-02 09:11:07 satoru Exp $
 * 
 * Copyright (C) 1997-1999 Satoru Takabayashi  All rights reserved.
 * This is free software with ABSOLUTELY NO WARRANTY.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA
 * 
 * This file must be encoded in EUC-JP encoding.
 * 
 */
%{
#include <string.h>
#include "namazu.h"
#include "util.h"
#include "field.h"
#include "output.h"
#include "re.h"
#include "result.h"
#include "em.h"

	void replace_field(uchar *);
	static int Fid, Did;
	static uchar Buf[BUFSIZE * 16];
	static int Score, Counter;
%}

%%
\$\{([-_a-zA-Z]+|namazu::[-_a-zA-Z]+)\} 	       replace_field(yytext);
.|\n                           strcat(Buf, yytext);
%%

/************************************************************
 *
 * Private functions
 *
 ************************************************************/

void make_fullpathname_result(int);
void encode_entity(uchar*);
void emphasize(uchar*);

void replace_field(uchar *key) 
{
    uchar field[BUFSIZE];
    /* 8 is length of '&quot;' + 2 (for emphasizing). 
       It's a consideration for buffer overflow (overkill?) */
    uchar buf[BUFSIZE * 8];  

    /* Remove '{$' and '}' */
    strcpy(field, key + 2);
    *(field + strlen(field) - 1) = '\0';

    if (strcmp(field, "namazu::score") == 0) {
	sprintf(buf, "%d", Score);
	commas(buf);
    } else if (strcmp(field, "namazu::counter") == 0) {
	sprintf(buf, "%d", Counter);
	commas(buf);
    } else {
	get_field_data(Did, Fid, field, buf);
    }
    /* do not emphasize in URI */
    if (strcmp(field, "uri") != 0 && HtmlOutput) {
	emphasize(buf);
    }
    if (HtmlOutput) {
	encode_entity(buf);
    }

    /* Insert commas if the buf is a numeric string */
    if (isnumstr(buf))
    {
	commas(buf);
    }

    strcat(Buf, buf);
}

void make_fullpathname_result(int n)
{
    uchar *base;

    base = Idx.names[n];
    pathcat(base, NMZ.result);
}

void encode_entity(uchar *str)
{
    int i;
    uchar tmp[BUFSIZE];

    strcpy(tmp, str);
    strcpy(str, "");
    for (i = 0; tmp[i]; i++) {
	if (tmp[i] == '<') {
	    strcat(str, "&lt;");
	} else if (tmp[i] == '>') {
	    strcat(str, "&gt;");
	} else if (tmp[i] == '&') {
	    strcat(str, "&amp;");
	} else if (tmp[i] == '"') {
	    strcat(str, "&quot;");
	} else {
	    strncat(str, tmp + i, 1);
	}
    }
}

/* inefficient algorithm but it works */
void emphasize(uchar *str)
{
    int i;

    for (i = 0; Query.tab[i] != NULL; i++) {
	uchar *ptr = str;
	uchar key[BUFSIZE];
	int keylen = 0;

	strcpy(key, Query.tab[i]);

	if (strchr(key, '\t')) { /* for phrase search */
	    tr(key, "\t", " ");
	    strcpy(key, key + 1); 
	    *(lastc(key)) = '\0';
	}

	keylen = strlen(key);

	do {
	    ptr = strcasestr(ptr, key);
	    if (ptr != NULL) {
		memmove(ptr + 2, ptr, strlen(ptr) + 1);
		memmove(ptr + 1, ptr + 2, keylen);
		*ptr = EM_START_MARK;
		*(ptr + keylen + 1) = EM_END_MARK;
		ptr += 2;
	    }
	} while (ptr != NULL);
    }
}

/************************************************************
 *
 * Public functions
 *
 ************************************************************/

/* display the hlist */
void print_hlist(HLIST hlist)
{
    int i;

    if (hlist.n <= 0 || HListMax == 0) {
	return;
    }

    for (i = HListWhence; i < hlist.n; i++) {
	char template[BUFSIZE];
	strcpy(Buf, "\t");  /* '\t' has an important role cf. fputx() */
	Score   = hlist.scr[i];
	Counter = i + 1;
	Fid = hlist.fid[i];
	Did = hlist.did[i];
	make_fullpathname_result(Did);
	strcpy(template, NMZ.result);
	strcat(template, ".");
	strcat(template, Template);  /* usually "normal" */

	if (!AllList && (i >= HListWhence + HListMax))
	    break;
	if (MoreShortFormat) {
	    get_field_data(Did, Fid, "uri", Buf);
	} else {
	    if ((yyin = fopen(template, "r")) == NULL) {
		die("print_hlist: %s", template);
		exit(-1);
	    }
	    yylex();
	    fclose(yyin);
	}
	fputx(Buf, stdout);
	fputx("\n", stdout);
    }
}

