/*
 * 
 * values.c -
 * 
 * $Id: values.c,v 1.7 1999-08-23 12:37:11 satoru Exp $
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
 * This file must be encoded in EUC-JP encoding.
 * 
 */

#include <stdio.h>
#include "namazu.h"

/* default directory of where indices are */
#ifdef INDEXDIR
uchar DEFAULT_DIR[BUFSIZE] = INDEXDIR;
#else
uchar DEFAULT_DIR[BUFSIZE] = "/usr/local/namazu/index";
#endif

/* string of HTML 's <BASE HREF="...">  (namazu.conf) */
uchar BASE_URI[BUFSIZE] = "";

/* namazu.conf */
#ifdef OPT_NAMAZU_CONF
uchar NAMAZU_CONF[BUFSIZE] = OPT_NAMAZU_CONF;
#else
uchar NAMAZU_CONF[BUFSIZE] = "/usr/local/namazu/lib/namazu.conf";
#endif

/* namazurc */
uchar NAMAZURC[BUFSIZE] = "";

/* files which Namazu uses */
uchar INDEX[BUFSIZE] = "NMZ.i";
uchar INDEXINDEX[BUFSIZE] = "NMZ.ii";
uchar HEADERFILE[BUFSIZE] = "NMZ.head";
uchar FOOTERFILE[BUFSIZE] = "NMZ.foot";
uchar LOCKFILE[BUFSIZE] = "NMZ.lock";
uchar BODYMSGFILE[BUFSIZE] = "NMZ.body";
uchar RESULTFILE[BUFSIZE] = "NMZ.result";
uchar SLOG[BUFSIZE] = "NMZ.slog";
uchar WORDLIST[BUFSIZE] = "NMZ.w";
uchar FIELDINFO[BUFSIZE] = "NMZ.field.";  /* followed by field name */
uchar DATEINDEX[BUFSIZE] = "NMZ.t"; 

uchar PHRASE[BUFSIZE] = "NMZ.p";
uchar PHRASEINDEX[BUFSIZE] = "NMZ.pi";

/* too much global variables */
int HListMax = 20;		/* max number of search results */
int HListWhence = 0;		/* number which beginning of search results */
int Debug = 0;			/* if debug mode is on: 1 */
int ShortFormat = 0;		/* if no display summary: 1  */
int MoreShortFormat = 0;        /* if more short format mode: 1  */
int Quiet = 0;                  /* if quiet mode: 1  */
int HitCountOnly = 0;
int ScoreSort = 1;		/* if sort by score: 1 */
int HtmlOutput = 1;		/* if display as HTML: 1 */
int HidePageIndex = 0;		/* if hide page index: 1 */
int ForcePrintForm = 0;		/* if display <FORM> ... </FORM>: 1 */
int AllList = 0;		/* if dispal all search results: 1 */
int LaterOrder = 1;		/* if sort by late order: 1 */
int ConfLoaded = 0;		/* if whould loaded namazu.conf: 1 */
int NoReplace = 0;		/* if no replace URI: 1 */
int DecodeURI = 0;		/* if replace URI at plaint text mode: 1 */
int IsCGI = 0;			/* if CGI mode: 1 */
int Logging = 1;		/* if do logging:  1 */
int DbNumber = 0;               /* number of databases */
int OppositeEndian = 0;         /* if opposite byte order: 1 */
int NoReference = 0;		/* if no display reference: 1  */

#ifdef SCORING
#define TFIDF  1
#define SIMPLE 0
int TfIdf = SCORING;   /* switch of TfIdf mode */
#else
int TfIdf = 1;
#endif

int AllDocumentN = 0;  /* number of all of documents */

uchar KeyTable[BUFSIZE];		/* table saving queries */
uchar *KeyItem[KEY_ITEM_MAX + 1];	/* pointers of items of query */
uchar *DbNames[DB_MAX + 1];   /* name of databases */
FILE *Index, *IndexIndex, *Phrase, *PhraseIndex;

REPLACE Replace = { NULL, NULL };
ALIAS   *Alias   = NULL;
