/*
 * 
 * libnamazu.c - Namazu library api
 *
 * $Id: libnamazu.c,v 1.18 2000-01-06 03:44:29 satoru Exp $
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
#include "output.h"
#include "search.h"
#include "cgi.h"
#include "hlist.h"
#include "i18n.h"
#include "regex.h"
#include "var.h"
#include "alias.h"

static enum nmz_sort_method  sortmethod  = SORT_BY_SCORE;
static enum nmz_sort_order   sortorder   = DESCENDING;
static int  debugmode   = 0;
static int  loggingmode = 1;   /* do logging with NMZ.slog */
static char dyingmsg[BUFSIZE] = "";

/*
 *
 * Public functions
 *
 */

void 
codeconv_query(char *query)
{
    if (is_lang_ja()) {
        if (conv_ja_any_to_eucjp(query)) {
            zen2han(query);
        }
    }
}

void 
set_sortmethod(enum nmz_sort_method method)
{
    sortmethod = method;
}

enum nmz_sort_method 
get_sortmethod(void)
{
    return sortmethod;
}

void 
set_sortorder(enum nmz_sort_order order)
{
    sortorder = order;
}

enum nmz_sort_order 
get_sortorder(void)
{
    return sortorder;
}

void 
set_debugmode(int mode)
{
    debugmode = mode;
}

int 
is_debugmode(void)
{
    return debugmode;
}

void 
set_loggingmode(int mode)
{
    loggingmode = mode;
}

int 
is_loggingmode(void)
{
    return loggingmode;
}


void 
set_dyingmsg(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsnprintf(dyingmsg, BUFSIZE, fmt, args);
    va_end(args);

}

char *
get_dyingmsg(void)
{
    return dyingmsg;
}


