/*
 * result.c -
 * $Id: result.c,v 1.25 1999-12-03 07:53:52 masao Exp $
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
 * 
 */
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "namazu.h"
#include "util.h"
#include "field.h"
#include "output.h"
#include "re.h"
#include "result.h"
#include "em.h"
#include "codeconv.h"
#include "var.h"
#include "parser.h"


/*
 *
 * Private functions
 *
 */

static void encode_entity(char*);
static void emphasize(char*);
static void replace_field(HLIST_DATA, int, char*, char*);


static void replace_field(HLIST_DATA d, int counter, 
			  char *field, char *result)
{
    /* 8 is length of '&quot;' + 2 (for emphasizing). 
       It's a consideration for buffer overflow (overkill?) */
    char buf[BUFSIZE * 8];  

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
    if (strcasecmp(field, "uri") != 0 && is_htmlmode()) {
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

static void encode_entity(char *str)
{
    int i;
    char tmp[BUFSIZE];

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
static void emphasize(char *str)
{
    int i;

    for (i = 0; Query.tab[i] != NULL; i++) {
	char *ptr = str;
	char key[BUFSIZE];
	int keylen = 0;

	if (isop(Query.tab[i]))
	    continue;

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

/* public functions */

void make_fullpathname_result(int n)
{
    char *base;

    base = Idx.names[n];
    pathcat(base, NMZ.result);
}

void compose_result(HLIST_DATA d, int counter, 
			   char *template, char *r)
{
    char *p = template;
    char achars[BUFSIZE]; /* acceptable characters */

    strcpy(r, "\t");  /* '\t' has an important role cf. html_print() */

    strcpy(achars, FIELD_SAFE_CHARS);
    strcat(achars, ":");  /* for namazu::score, namazu::counter */

    do {
	char *pp;
	pp = strstr(p, "${");
	if (pp != NULL) {
	    int n;
	    strncat(r, p, pp - p);
	    pp += 2;  /* skip "${" */
	    n = strspn(pp, achars);
	    if (n > 0 && pp[n] == '}') {
		char field[BUFSIZE];
		
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

