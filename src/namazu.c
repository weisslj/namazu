/*
 * 
 * namazu.c - search client of Namazu
 *
 * $Id: namazu.c,v 1.100 2000-01-28 09:40:21 satoru Exp $
 * 
 * Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.
 * Copyright (C) 2000 Namazu Project All rights reserved.
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

/*
 *
 * Public functions
 *
 */

void 
set_templatedir(char *dir)
{
    strcpy(templatedir, dir);
}

char *
get_templatedir(void)
{
    return templatedir;
}

/* 
 * Print the error message and die.
 */
void 
die(const char *fmt, ...)
{
    va_list args;

    fflush(stdout);
    fflush(stderr);

    fprintf(stderr, "%s: ", PACKAGE);

    /*
     * Print a message specified by function parameters.
     */
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    if (fmt[strlen(fmt) - 1] != '\n') {
	printf("\n");
    }

    exit(EXIT_FAILURE);
}

/*
 * Namazu core routine.
 */
enum nmz_stat 
namazu_core(char * query, char *subquery)
{
    char query_with_subquery[BUFSIZE * 2];
    NmzResult hlist;

    /* Make full-pathname of NMZ.{head,foot,msg,body,slog}.?? */
    make_fullpathname_msg();

    strcpy(query_with_subquery, query);
    strcat(query_with_subquery, " ");
    strcat(query_with_subquery, subquery);

    /* 
     * If query is null or the number of indices are 0
     * show NMZ.head,body,foot and return with SUCCESS.
     */
    if (*query == '\0' || nmz_get_idxnum() == 0) {
	print_default_page();
	nmz_free_internal();
	return SUCCESS;
    }

    nmz_debug_printf(" -n: %d\n", get_maxresult());
    nmz_debug_printf(" -w: %d\n", get_listwhence());
    nmz_debug_printf("query: [%s]\n", query);

    /* Search */
    hlist = nmz_search(query_with_subquery);

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

