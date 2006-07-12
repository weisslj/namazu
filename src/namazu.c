/*
 * 
 * namazu.c - search client of Namazu
 *
 * $Id: namazu.c,v 1.109 2006-07-12 16:52:44 opengl2772 Exp $
 * 
 * Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.
 * Copyright (C) 2000-2006 Namazu Project All rights reserved.
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
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>
#include <signal.h>

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#else
# ifdef _WIN32
# include <io.h>
# endif
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include "namazu.h"
#include "libnamazu.h"
#include "util.h"
#include "codeconv.h"
#include "output.h"
#include "search.h"
#include "hlist.h"
#include "idxname.h"
#include "i18n.h"
#include "system.h"
#include "namazu.h"
#include "var.h"

static char templatedir[BUFSIZE] = "";

/*
 *
 * Private functions
 *
 */

static void make_fullpathname_msg ( void );
static void make_exquery( char *query, char *exquery, size_t sz, char *querymode, int ct_query );

static void 
make_fullpathname_msg(void)
{
    char *base;
    
    if (templatedir[0] != '\0') {
	/* 
	 * If templatedir is specified. 
	 * NOTE: templatedir can be set by namazurc's Template directive.
	 */
	base = templatedir;
    } else if (nmz_get_idxnum() == 1) {
	/* Only one index is targeted. */
        base = nmz_get_idxname(0);
    } else {
	/* Multiple indices are targeted. */
	/* As default, use defaultidx's template files. */
	base = nmz_get_defaultidx();
    }
    
    nmz_pathcat(base, NMZ.head);
    nmz_pathcat(base, NMZ.foot);
    nmz_pathcat(base, NMZ.body);
    nmz_pathcat(base, NMZ.lock);
    nmz_pathcat(base, NMZ.tips);
}

static void
make_exquery(char *query, char *exquery, size_t sz, char *querymode, int ct_query)
{
    char *p;

    if (exquery[0] != '\0') {
        strncat(exquery, " ", sz - strlen(exquery) - 1);
        exquery[sz - 1] = '\0';
    }

    p = querymode;
    if (!strncasecmp(p, "field:", strlen("field:"))) {
        char buff[BUFSIZE] = "";
        int simple = 1;
        char *s;

        s = query;
        while(*s) {
            if (*s == ' ') {
                simple = 0;
                break;
            }
            s++;
        }

        p += strlen("field:");
        if (simple) {
            snprintf(buff, BUFSIZE - 1, "+%s:%s", p, query);
        } else {
            snprintf(buff, BUFSIZE - 1, "+%s:\"%s\"", p, query);
        }
        buff[BUFSIZE - 1] = '\0';

        strncat(exquery, buff, sz - strlen(exquery) - 1);
        exquery[sz - 1] = '\0';
    } else {
        /* normal */
        if (ct_query == 1) {
            strncat(exquery, query, sz - strlen(exquery) - 1);
            exquery[sz - 1] = '\0';
        } else {
            strncat(exquery, "( ", sz - strlen(exquery) - 1);
            strncat(exquery, query, sz - strlen(exquery) - 1);
            strncat(exquery, " )", sz - strlen(exquery) - 1);
            exquery[sz - 1] = '\0';
        }
    }
}

/*
 *
 * Public functions
 *
 */

void 
set_templatedir(char *dir)
{
    strncpy(templatedir, dir, BUFSIZE - 1);
}

char *
get_templatedir(void)
{
    return templatedir;
}

/*
 * Namazu core routine.
 */
enum nmz_stat 
namazu_core(char * query, char *subquery)
{
    char *query_with_subquery = NULL;
    char *exquery = NULL;
    char *_query[NUM_QUERY];
    NmzResult hlist;
    int ct_query;
    int i;

    /* Make full-pathname of NMZ.{head,foot,msg,body,slog}.?? */
    make_fullpathname_msg();

    /* 
     * Convert query and subquery's  encoding for searching. 
     */
    nmz_codeconv_query(query);
    nmz_codeconv_query(subquery);

    /*
     * count query. and check query.
     */
    ct_query = 0;
    for(i = 0; i < NUM_QUERY; i++) {
        if (i == 0) {
            _query[0] = query;
        } else {
            _query[i] = nmz_get_query(i);
        }
        if (_query[i] && _query[i][0] != '\0') {
            ct_query++;
        }
    }

    /* 
     * If query is null or the number of indices is 0
     * show NMZ.head,body,foot and return with SUCCESS.
     */
#if 0
    if (*query == '\0' || nmz_get_idxnum() == 0) {
#else
    if (ct_query == 0 || nmz_get_idxnum() == 0) {
#endif
	print_default_page();
	nmz_free_internal();
	return SUCCESS;
    }

    if ((query_with_subquery = 
    (char *)calloc(sizeof(char), BUFSIZE * (NUM_QUERY + 1))) == NULL) {
	die("memory over");
    }
    if ((exquery = (char *)calloc(sizeof(char), BUFSIZE * NUM_QUERY)) == NULL) {
        free(query_with_subquery);
	die("memory over");
    }

    /*
     * query mode
     */
    exquery[0] = '\0';
    if (query[0] != '\0') {
        make_exquery(query, exquery, BUFSIZE * NUM_QUERY, nmz_get_querymode(0), ct_query);
    }
    {
        int i;

        for(i = 1; i < NUM_QUERY; i++) {
            if (_query[i] && _query[i][0] != '\0') {
                char buff[BUFSIZE] = "";

                strncpy(buff, _query[i], BUFSIZE - 1);
                buff[BUFSIZE - 1] = '\0';
                nmz_codeconv_query(buff);
                make_exquery(buff, exquery, BUFSIZE * NUM_QUERY, nmz_get_querymode(i), ct_query);
            }
        }
    }

    /*
     * And then, concatnate them.
     */
    if (strlen(subquery) > 0) {
        strncpy(query_with_subquery, "( ", BUFSIZE * (NUM_QUERY + 1) - 1);
        strncat(query_with_subquery, exquery, BUFSIZE * (NUM_QUERY + 1) - 1 - strlen(query_with_subquery));
	strncat(query_with_subquery, " ) ", BUFSIZE * (NUM_QUERY + 1) - 1 - strlen(query_with_subquery));
	strncat(query_with_subquery, subquery, BUFSIZE * (NUM_QUERY + 1) - 1 - strlen(query_with_subquery));
    } else {
        strncpy(query_with_subquery, exquery, BUFSIZE * (NUM_QUERY + 1) - 1);
    }

    nmz_debug_printf(" -n: %d\n", get_maxresult());
    nmz_debug_printf(" -w: %d\n", get_listwhence());
    nmz_debug_printf("query: [%s]\n", query);
    nmz_debug_printf("exquery: [%s]\n", exquery);
    nmz_debug_printf("querymode: [%s]\n", nmz_get_querymode(0));

    free(exquery);

    /* Search */
    hlist = nmz_search(query_with_subquery);

    free(query_with_subquery);

    if (hlist.stat == ERR_FATAL) {
	die(nmz_get_dyingmsg());
    }

    /* Result printing */
    if (print_result(hlist, query, subquery) != SUCCESS) {
	return FAILURE;
    }

    nmz_free_hlist(hlist);
    nmz_free_internal();

    return SUCCESS;
}

