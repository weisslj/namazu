/*
 * 
 * $Id: query.c,v 1.7 2000-01-20 02:54:42 satoru Exp $
 * 
 * Copyright (C) 1997-2000 Satoru Takabayashi  All rights reserved.
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

#include <stdlib.h>
#include <string.h>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "libnamazu.h"
#include "util.h"
#include "var.h"
#include "field.h"
#include "query.h"

static struct nmz_query query = {0}; /* Initialize member `tokennum' with 0 */

/*
 *
 * Private functions
 *
 */

static void set_phrase_trick(char *str);
static void set_regex_trick(char *str);

/* 
 * Replace duble quotes with spaces and replace internal spaces with TABs
 *{foo bar} is also acceptable.
 * FIXME: very complicated ad hoc routine.
 */
static void 
set_phrase_trick(char *str)
{
    int i, state;
    char *b = str, *e;

    for (i = state = 0; *(str + i); i++) {
        if ((*(str + i) == '"' || *(str + i) == '{') 
            && (i == 0 || *(str + i - 1) == ' ')) 
        {
            state = 1;
            b = str + i + 1;
        } else if (state && (*(str + i) == '"' || *(str + i) == '}') && 
                   (*(str + i + 1) == ' ' || *(str + i + 1) == '\0')) 
        {
            state = 0;
            e = str + i - 1;

            for (;b <= e; b++) {
                if (*b == ' ')
                    *b = '\t';
            }
        } 
    }
}

/* 
 * Replace internal spaces in the regex pattern with  
 * FIXME: very complicated ad hoc routine.
 */
static void 
set_regex_trick(char *str)
{
    int i, delim;
    char *b = str, *e;

    for (i = delim = 0; *(str + i); i++) {
        int field = 0;
        if ((i == 0 || *(str + i - 1) == ' ') && nmz_isfield(str + i)) {
            field = 1;
            i += strcspn(str + i, ":") + 1;
        }
        if ((field || i == 0 || *(str + i - 1) == ' ') && 
            (*(str + i) == '/' || 
             (field && (*(str + i) == '"' || *(str + i) == '{'))))
        {
            delim = *(str + i);
            if (delim == '{') {
                delim = '}';
            }
            b = str + i + 1;
        } else if (*(str + i) == delim 
                   && (*(str + i + 1) == ' ' || *(str + i + 1) == '\0')) 
        {
            delim = 0;
            e = str + i - 1;

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

/*
 * Make the query from the string querystring.
 * FIXME: The function is tremendously dirty. it should be rewritten.
 */
enum nmz_stat
nmz_make_query(const char *querystring)
{
    int i, tokennum;

    if (strlen(querystring) > QUERY_MAX) {
	return ERR_TOO_LONG_QUERY;
    }

    strcpy(query.str, querystring);
    /* Normalize the query.str encoding for searching. */
    nmz_codeconv_query(query.str);

    set_phrase_trick(query.str);
    set_regex_trick(query.str);

    /* Count number of tokens in querystring. */
    for (i = 0, tokennum = 0; *(query.str + i);) {
	while (query.str[i] == ' ')
	    i++;
	if (query.str[i])
	    tokennum++;
	while (query.str[i] != ' ' &&
	       query.str[i] != '\0')
	    i++;
    }
    if (nmz_is_debugmode()) {
	nmz_debug_printf("query.tabN: %d\n", tokennum);
    }

    if (tokennum == 0) { /* if no token available */
	return ERR_INVALID_QUERY;
    }

    /* If too much items in query, return with error */
    if (tokennum > QUERY_TOKEN_MAX) {
	return ERR_TOO_MANY_TOKENS;
    }

    /* Assign a pointer to each token and set NULL to the end of each token. */
    for (i = 0, tokennum = 0; query.str[i];) {
	while (query.str[i] == ' ')
	    i++;
	if (query.str[i])
	    query.tab[tokennum++] = &query.str[i];
	while (query.str[i] != ' ' &&
	       query.str[i] != '\0')
	    i++;
	if (query.str[i] != '\0')
	    query.str[i++] = '\0';
    }

    /* Set NULL to the last key table. */
    query.tab[tokennum] = (char *) NULL;

    /* Replace  with spaces (restore). */
    for (i = 0; i < tokennum; i++) {
	nmz_tr(query.tab[i], "", " ");
    }

    /* Assign tokennum. */
    query.tokennum = tokennum;

    return SUCCESS;
}

int
nmz_get_querytokennum(void)
{
    return query.tokennum;
}

char *
nmz_get_querytoken(int id)
{
    return query.tab[id];
}
