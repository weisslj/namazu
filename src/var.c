/*
 * 
 * values.c -
 * 
 * $Id: var.c,v 1.10 1999-11-01 14:13:20 satoru Exp $
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
#include "namazu.h"
#include "re.h"

/* string of HTML 's <BASE HREF="...">  (namazu.conf) */
uchar BASE_URI[BUFSIZE] = "";

/* default directory to place indices */
uchar DEFAULT_INDEX[BUFSIZE] = OPT_INDEXDIR;

/* default directory to place config files */
uchar CONFDIR[BUFSIZE] = OPT_CONFDIR;

/* namazurc */
uchar NAMAZURC[BUFSIZE] = "";

/* too many global variables */
int HListMax = 20;		/* max number of search results */
int HListWhence = 0;		/* number which beginning of search results */
int Debug = 0;			/* if debug mode is on: 1 */
int ListFormat = 0;        /* if more short format mode: 1  */
int Quiet = 0;                  /* if quiet mode: 1  */
int HitCountOnly = 0;
int HtmlOutput = 0;		/* if display as HTML: 1 */
int HidePageIndex = 0;		/* if hide page index: 1 */
int ForcePrintForm = 0;		/* if display <FORM> ... </FORM>: 1 */
int AllList = 0;		/* if dispal all search results: 1 */
int NoReplace = 0;		/* if no replace URI: 1 */
int DecodeURI = 0;		/* if replace URI at plaint text mode: 1 */
int IsCGI = 0;			/* if CGI mode: 1 */
int Logging = 1;		/* if do logging:  1 */
int NoReference = 0;		/* if no display reference: 1  */
int SortMethod = SORT_BY_SCORE;	/* Method of sorting */
int SortOrder = DESCENDING;	/* Direction of sorting */


#ifdef OPT_SCORING
#define TFIDF  1
#define SIMPLE 0
int TfIdf = OPT_SCORING;   /* switch of TfIdf mode */
#else
int TfIdf = 1;
#endif

REPLACE *Replace = NULL;
ALIAS   *Alias   = NULL;

NMZ_FILES Nmz;  /* NMZ.* files' file pointers */
NMZ_NAMES NMZ = {  /* NMZ.* files' names */
    "NMZ.i", 
    "NMZ.ii",
    "NMZ.head",
    "NMZ.foot",
    "NMZ.body",
    "NMZ.lock",
    "NMZ.result",
    "NMZ.slog",
    "NMZ.w",
    "NMZ.wi",
    "NMZ.field.",
    "NMZ.t",
    "NMZ.p",
    "NMZ.pi",
    "NMZ.tips",
    "NMZ.access"
};

INDICES Idx;
QUERY Query;

char Template[BUFSIZE] = "normal";
char Dyingmessage[BUFSIZE] = "Initialized";
