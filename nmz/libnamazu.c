/*
 * 
 * libnamazu.c - Namazu library api
 *
 * $Id: libnamazu.c,v 1.11 1999-12-04 07:32:23 satoru Exp $
 * 
 * Copyright (C) 1997-1999 Satoru Takabayashi  All rights reserved.
 * Copyright (C) 1999 NOKUBI Takatsugu All rights reserved.
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
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdarg.h>

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "namazu.h"
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "namazu.h"
#include "libnamazu.h"
#include "getopt.h"
#include "util.h"
#include "codeconv.h"
#include "form.h"
#include "usage.h"
#include "conf.h"
#include "output.h"
#include "search.h"
#include "cgi.h"
#include "hlist.h"
#include "i18n.h"
#include "regex.h"
#include "var.h"

static enum nmz_sort_method  sortmethod  = SORT_BY_SCORE;
static enum nmz_sort_order   sortorder   = DESCENDING;
static int  debugmode   = 0;
static int  loggingmode = 1;   /* do logging with NMZ.slog */
static char dyingmsg[BUFSIZE] = "Initialized";

/*
 *
 * Public functions
 *
 */

void free_idxnames(void)
{
    int i;
    for (i = 0; i < Idx.num; i++) {
        free(Idx.names[i]);
	free_hitnums(Idx.pr[i]);
    }
    Idx.num = 0;
}

void free_aliases(void)
{
    struct nmz_alias *list, *next;
    list = Alias;

    while (list) {
	next = list->next;
	free(list->alias);
	free(list->real);
	free(list);
	list = next;
    }
}

void free_replaces(void)
{
    struct nmz_replace *list, *next;
    list = Replace;

    while (list) {
	next = list->next;
	free(list->pat);
	free(list->rep);
	if (list->pat_re != NULL) {
	    re_free_pattern(list->pat_re);
	}
	free(list);
	list = next;
    }
}

void make_fullpathname_msg(void)
{
    char *base;
    
    if (Idx.num == 1) {
        base = Idx.names[0];
    } else {
        base = DEFAULT_INDEX;
    }
    
    pathcat(base, NMZ.head);
    pathcat(base, NMZ.foot);
    pathcat(base, NMZ.body);
    pathcat(base, NMZ.lock);
    pathcat(base, NMZ.tips);
}

void codeconv_query(char *query)
{
    if (is_lang_ja()) {
        if (conv_ja_any_to_eucjp(query)) {
            zen2han(query);
        }
    }
}

/* get an environmental variable of NAMAZUCONFPATH
 * original by Shimizu-san [1998-02-27]
 */
void getenv_namazurc(void)
{
    char *env_namazu_conf;

    env_namazu_conf = getenv("NAMAZUCONFPATH");
    if (env_namazu_conf == NULL)
        env_namazu_conf = getenv("NAMAZUCONF");

    if (env_namazu_conf != NULL)
        strcpy(NAMAZURC, env_namazu_conf);
}

void uniq_idxnames(void)
{
    int i, j, k;

    for (i = 0; i < Idx.num - 1; i++) {
        for (j = i + 1; j < Idx.num; j++) {
            if (strcmp(Idx.names[i], Idx.names[j]) == 0) {
                free(Idx.names[j]);
                for (k = j + 1; k < Idx.num; k++) {
                    Idx.names[k - 1] = Idx.names[k];
                }
                Idx.num--;
                j--;
            }
        }
    }
}

int expand_idxname_aliases(void)
{
    int i;

    for (i = 0; i < Idx.num; i++) {
	struct nmz_alias *list = Alias;
	while (list) {
	    if (strcmp(Idx.names[i], list->alias) == 0) {
		free(Idx.names[i]);
		Idx.names[i] = (char *) malloc(strlen(list->real) + 1);
		if (Idx.names[i] == NULL) {
		    set_dyingmsg("expand_idxname_aliases: malloc()");
		    return FAILURE;
		}
		strcpy(Idx.names[i], list->real);
            }
	    list = list->next;
	}
    }
    return 0;
}

int complete_idxnames(void)
{
    int i;

    for (i = 0; i < Idx.num; i++) {
 	if (*Idx.names[i] == '+' && isalnum(*(Idx.names[i] + 1))) {
	    char *tmp;
	    tmp = (char *)malloc(strlen(DEFAULT_INDEX) 
				  + 1 + strlen(Idx.names[i]) + 1);
	    if (tmp == NULL) {
		set_dyingmsg("complete_idxnames: malloc()");
		return FAILURE;
	    }
	    strcpy(tmp, DEFAULT_INDEX);
	    strcat(tmp, "/");
	    strcat(tmp, Idx.names[i] + 1);  /* +1 means '+' */
	    free(Idx.names[i]);
	    Idx.names[i] = tmp;
	}
    }
    return 0;
}

char *set_namazurc(char *arg)
{
    return strcpy(NAMAZURC, arg);
}

void set_sortmethod(enum nmz_sort_method method)
{
    sortmethod = method;
}

enum nmz_sort_method get_sortmethod(void)
{
    return sortmethod;
}

void set_sortorder(enum nmz_sort_order order)
{
    sortorder = order;
}

enum nmz_sort_order get_sortorder(void)
{
    return sortorder;
}

void set_debugmode(int mode)
{
    debugmode = mode;
}

int is_debugmode(void)
{
    return debugmode;
}

void set_loggingmode(int mode)
{
    loggingmode = mode;
}

int is_loggingmode(void)
{
    return loggingmode;
}


void set_dyingmsg(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsnprintf(dyingmsg, BUFSIZE, fmt, args);
    va_end(args);

}

char *get_dyingmsg(void)
{
    return dyingmsg;
}


