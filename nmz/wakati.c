/*
 * 
 * wakati.c -
 * 
 * $Id: wakati.c,v 1.11 1999-12-09 03:15:15 satoru Exp $
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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "namazu.h"
#include "libnamazu.h"
#include "message.h"
#include "util.h"
#include "field.h"
#include "codeconv.h"
#include "search.h"
#include "output.h"
#include "wakati.h"
#include "i18n.h"
#include "var.h"


/*
 *
 * Private functions
 *
 */

static int detect_char_type(char*);
static void set_phrase_trick(char*);
static void set_regex_trick(char*);

static int detect_char_type(char *c)
{
    if (iskatakana(c)) {
        return KATAKANA;
    } else if (ishiragana(c)){
        return HIRAGANA;
    } else if (iskanji(c)) {
        return KANJI;
    }
    return OTHER;
}

/* 
 * replace duble quotes with spaces and replace internal spaces with TABs
 *{foo bar} is also acceptable 
 */
static void set_phrase_trick(char *qs)
{
    int i, state;
    char *b = qs, *e;

    for (i = state = 0; *(qs + i); i++) {
        if ((*(qs + i) == '"' || *(qs + i) == '{') 
            && (i == 0 || *(qs + i - 1) == ' ')) 
        {
            state = 1;
            b = qs + i + 1;
        } else if (state && (*(qs + i) == '"' || *(qs + i) == '}') && 
                   (*(qs + i + 1) == ' ' || *(qs + i + 1) == '\0')) 
        {
            state = 0;
            e = qs + i - 1;

            for (;b <= e; b++) {
                if (*b == ' ')
                    *b = '\t';
            }
        } 
    }
}

/* 
 * replace internal spaces with  
 * very complicated ad hoc routine :-( 
 */
static void set_regex_trick(char *qs)
{
    int i, delim;
    char *b = qs, *e;

    for (i = delim = 0; *(qs + i); i++) {
        int field = 0;
        if ((i == 0 || *(qs + i - 1) == ' ') && isfield(qs + i)) {
            field = 1;
            i += strcspn(qs + i, ":") + 1;
        }
        if ((field || i == 0 || *(qs + i - 1) == ' ') && 
            (*(qs + i) == '/' || 
             (field && (*(qs + i) == '"' || *(qs + i) == '{'))))
        {
            delim = *(qs + i);
            if (delim == '{') {
                delim = '}';
            }
            b = qs + i + 1;
        } else if (*(qs + i) == delim 
                   && (*(qs + i + 1) == ' ' || *(qs + i + 1) == '\0')) 
        {
            delim = 0;
            e = qs + i - 1;

            for (;b <= e; b++) {
                if (*b == ' ')
                    *b = '';
            }
        } 
    }
}

/*
 *
 * Public functions
 *
 */

int wakati(char *key)
{
    int i, j, key_leng, type;
    char buf[BUFSIZE * 2] = "";

    nmz_dprintf("wakati original: [%s].\n", key);

    for (i = 0; i < strlen(key); ) {
        type = detect_char_type(key + i);
	if (iseuc(*(key + i))) {
	    key_leng = 0;
	    for (j = 0; iskanji(key + i + j) ;  j += 2) {
		char tmp[BUFSIZE];

                if (j == 0 && (iskatakana(key + i + j) ||
                    ishiragana(key + i + j))) 
                {
                    /* if beggining character is Katakana or Hiragana */
                    break;
                }

		strncpy(tmp, key + i, j + 2);
		*(tmp + j + 2) = '\0';

		if (binsearch(tmp, 0) != -1) {
		    key_leng = j + 2;
		}
	    }

	    if (key_leng > 0) {
		strncat(buf, key + i, key_leng); 
                strcat(buf, "\t");
		i += key_leng;
	    } else {
                if (type == HIRAGANA || type == KATAKANA) {
                    for (j =0; ; j += 2) {
                        if (!((type == HIRAGANA && ishiragana(key + i + j))
                            ||(type == KATAKANA && iskatakana(key + i + j)))) 
                        {
                            break;
                        }
                        strncat(buf, key + i + j, 2);
                    }
                    i += j;
                    strcat(buf, "\t");
                } else {
                    strncat(buf, key + i, 2);
                    strcat(buf, "\t");
                    i += 2;
                }
	    }
	} else {
            while(*(key + i) && !iseuc(*(key + i))) {
                /* as an initial attempt always success, 
                   outer 'for loop' can avoid infinite loop */
                strncat(buf, key + i, 1);
                i++;
            }
            strcat(buf, "\t");
	}
    }
    nmz_chomp(buf);

    if (strlen(buf) <= BUFSIZE) {
	strcpy(key, buf);
    } else {
	set_dyingmsg("wakatigaki processing failed.\n");
	return 1; 
    }
    nmz_dprintf("wakatied string: [%s]\n", key);
    return 0;
}


/* split a given query */
int split_query(char *qs)
{
    int i, qn;

    set_phrase_trick(qs);
    set_regex_trick(qs);

    if (strlen(qs) >= BUFSIZE - 1) {
	return ERR_TOO_LONG_QUERY;
    }

    strcpy(Query.str, qs);

    /* count items in query */
    for (i = 0, qn = 0; *(Query.str + i);) {
	while (Query.str[i] == ' ')
	    i++;
	if (Query.str[i])
	    qn++;
	while (Query.str[i] != ' ' &&
	       Query.str[i] != '\0')
	    i++;
    }
    if (is_debugmode()) {
	nmz_dprintf("Query.tabN: %d\n", qn);
    }

    if (qn == 0) { /* if no item available */
	return ERR_INVALID_QUERY;
    }

    /* if too much items in query, return with error */
    if (qn > QUERY_TOKEN_MAX) {
	return ERR_TOO_MANY_TOKENS;
    }
    /* assign a pointer to each item and set NULL to the last of table */
    for (i = 0, qn = 0; Query.str[i];) {
	while (Query.str[i] == ' ')
	    i++;
	if (Query.str[i])
	    Query.tab[qn++] = &Query.str[i];
	while (Query.str[i] != ' ' &&
	       Query.str[i] != '\0')
	    i++;
	if (Query.str[i])
	    Query.str[i++] = '\0';
    }

    /* set NULL to the last key item */
    Query.tab[qn] = (char *) NULL;

    /* replace  with spaces */
    for (i = 0; i < qn; i++) {
	nmz_tr(Query.tab[i], "", " ");
    }
    return SUCCESS;
}




