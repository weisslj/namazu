/*
 * 
 * libnamazu.c - Namazu library api
 *
 * $Id: libnamazu.c,v 1.30 2000-02-01 04:38:28 rug Exp $
 * 
 * Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.
 * Copyright (C) 2000 Namazu Project All rights reserved.
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
#include "libnamazu.h"
#include "util.h"
#include "codeconv.h"
#include "search.h"
#include "hlist.h"
#include "i18n.h"
#include "regex.h"
#include "var.h"
#include "alias.h"
#include "replace.h"
#include "idxname.h"

static enum nmz_sortmethod  sortmethod  = SORT_BY_SCORE;
static enum nmz_sortorder   sortorder   = DESCENDING;
static int  debugmode   = 0;
static int  loggingmode = 1;   /* do logging with NMZ.slog */
static char dyingmsg[BUFSIZE] = "";


/*
 *
 * Public functions
 *
 */


/*
 * Free all internal data.
 */ 
void
nmz_free_internal(void)
{
    nmz_free_idxnames();
    nmz_free_aliases();
    nmz_free_replaces();
}

void 
nmz_set_sortmethod(enum nmz_sortmethod method)
{
    sortmethod = method;
}

enum nmz_sortmethod 
nmz_get_sortmethod(void)
{
    return sortmethod;
}

void 
nmz_set_sortorder(enum nmz_sortorder order)
{
    sortorder = order;
}

enum nmz_sortorder 
nmz_get_sortorder(void)
{
    return sortorder;
}

void 
nmz_set_debugmode(int mode)
{
    debugmode = mode;
}

int 
nmz_is_debugmode(void)
{
    return debugmode;
}

void 
nmz_set_loggingmode(int mode)
{
    loggingmode = mode;
}

int 
nmz_is_loggingmode(void)
{
    return loggingmode;
}

/*
 * This function is used for formating a string with printf
 * notation and store the string in the static variable
 * `msg'.  and return its pointer. So, thhe string can only
 * be used until the next call to the function.  
 *
 * NOTE: Mainly used with nmz_set_dyingmsg() macro.
 */
char *
nmz_msg(const char *fmt, ...)
{
    static char msg[BUFSIZE];
    va_list args;
    
    va_start(args, fmt);
    vsnprintf(msg, BUFSIZE, fmt, args);
    va_end(args);

    return msg;
}

/*
 * This function is not used directly but used only through
 * nmz_set_dyingmsg() macro. That's for getting __FILE__ and
 * __LINE__ information and including them in the
 * dyingmsg in debug mode. It makes debug easy.  
 */
char *
nmz_set_dyingmsg_sub(const char *fmt, ...)
{
    va_list args;
    
    va_start(args, fmt);
    vsnprintf(dyingmsg, BUFSIZE, fmt, args);
    va_end(args);

    return dyingmsg;
}


char *
nmz_get_dyingmsg(void)
{
    return dyingmsg;
}


