/*
 * result.l -
 * -*- C -*-
 * $Id: result.c,v 1.16 1999-09-06 01:13:11 satoru Exp $
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
#include <string.h>
#include <stdlib.h>
#include "namazu.h"
#include "util.h"
#include "field.h"
#include "output.h"
#include "re.h"
#include "result.h"
#include "em.h"

/************************************************************
 *
 * Private functions
 *
 ************************************************************/

void make_fullpathname_result(int);
void encode_entity(uchar*);
void emphasize(uchar*);
void compose_result(hlist_data, int, uchar*, uchar*);
void replace_field(hlist_data, int, uchar*, uchar*);

void compose_result(hlist_data d, int counter, uchar *template, uchar *r)
{
    uchar *p = template;
    uchar achars[BUFSIZE]; /* acceptable characters */

    strcpy(r, "\t");  /* '\t' has an important role cf. fputx() */

    strcpy(achars, FIELD_SAFE_CHARS);
    strcat(achars, ":");  /* for namazu::score, namazu::counter */

    do {
	uchar *pp;
	pp = strstr(p, "${");
	if (pp != NULL) {
	    int n;
	    strncat(r, p, pp - p);
	    pp += 2;  /* skip "${" */
	    n = strspn(pp, achars);
	    if (n > 0 && pp[n] == '}') {
		uchar field[BUFSIZE];
		
		strncpy(field, pp, n);
		field[n] = '\0';
		replace_field(d, counter, field, r);
		p = pp + n + 1; /* +1 for skipping "}" */
	    } else {
		p += 2;
	    }
	} else {
	    strcat(r, p);
	    break;
	}
    } while (1);
}


void replace_field(hlist_data d, int counter, uchar *field, uchar *result)
{
    /* 8 is length of '&quot;' + 2 (for emphasizing). 
       It's a consideration for buffer overflow (overkill?) */
    uchar buf[BUFSIZE * 8];  

    if (strcmp(field, "namazu::score") == 0) {
	sprintf(buf, "%d", d.score);
	commas(buf);
    } else if (strcmp(field, "namazu::counter") == 0) {
	sprintf(buf, "%d", counter);
	commas(buf);
    } else {
	get_field_data(d.idxid, d.docid, field, buf);
    }
    /* do not emphasize in URI */
    if (strcasecmp(field, "uri") != 0 && HtmlOutput) {
	emphasize(buf);
    }
    encode_entity(buf);

    /* Insert commas if the buf is a numeric string */
    if (isnumstr(buf))
    {
	commas(buf);
    }

    strcat(result, buf);
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
    uchar *templates[INDEX_MAX];
    uchar result[BUFSIZE * 16];

    if (hlist.n <= 0 || HListMax == 0) {
	return;
    }

    /* set NULL to all templates[] */
    for (i = 0; i < Idx.num; i++) {
	templates[i] = NULL;
    }

    for (i = HListWhence; i < hlist.n; i++) {
	int counter;

	counter = i + 1;

	if (!AllList && (i >= HListWhence + HListMax))
	    break;
	if (MoreShortFormat) {
	    get_field_data(hlist.d[i].idxid, hlist.d[i].docid, "uri", result);
	} else {
	    if (templates[hlist.d[i].idxid] == NULL) {  /* not loaded */
		uchar fname[BUFSIZE];

		make_fullpathname_result(hlist.d[i].idxid);
		strcpy(fname, NMZ.result);
		strcat(fname, ".");
		strcat(fname, Template);  /* usually "normal" */
		templates[hlist.d[i].idxid] = readfile(fname);
	    }
	    compose_result(hlist.d[i], counter, 
			   templates[hlist.d[i].idxid],  result);
	}
	fputx(result, stdout);
	fputx("\n", stdout);
    }

    /* free all templates[] */
    for (i = 0; i < Idx.num; i++) {
	if (templates[i] != NULL) {
	    free(templates[i]);
	}
    }
}

