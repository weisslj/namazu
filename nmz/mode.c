/*
 * 
 * mode.c - Namazu mode controlling class.
 * 
 * $Id: mode.c,v 1.1 1999-11-19 08:08:10 satoru Exp $
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "namazu.h"
#include "critical.h"
#include "magic.h"

/*
 *
 * default settings
 *
 */

static int sortmethod  = SORT_BY_SCORE;
static int sortorder   = DESCENDING;

static int maxlistnum  = 20;  /* max number of search results */
static int listwhence  = 0;   /* number which beginning of search results */
		      
static int htmlmode    = 0;
static int cgimode     = 0;
static int quietmode   = 0;
static int debugmode   = 0;
static int loggingmode = 1;   /* do logging with NMZ.slog */
		      
static int count_only  = 0;   /* like grep -c */
static int list_format = 0;   /* like grep -l */
		      
static int all_result  = 0;   /* print all results */
static int page_index  = 0;   /* print "Page: [1][2][3][4][5][6][7][8]" */
static int form_print  = 1;   /* print "<form> ... </form>" at cgimode */
static int ref_print   = 1;   /* print "References:  [ foo: 123 ]" */
		      
static int uri_replace = 1;   /* replace URI in results */
static int uri_decode  = 0;   /* decode URI in results */

static char template[BUFSIZE]     = "normal"; /* suffix of NMZ.result.* */
static char dyingmessage[BUFSIZE] = "Initialized";


/*
 *
 * Public functions
 *
 */

void set_htmlmode(int mode)
{
    htmlmode = mode;
}

int is_htmlmode(void)
{
    return htmlmode;
}

void set_sortmethod(int method)
{
    sortmethod = method;
}

int get_sortmethod(void)
{
    return sortmethod;
}

