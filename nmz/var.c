/*
 * 
 * var.c -
 * 
 * $Id: var.c,v 1.9 1999-12-04 07:32:23 satoru Exp $
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
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "namazu.h"
#include "libnamazu.h"
#include "re.h"

/* string of HTML 's <BASE HREF="...">  (namazu.conf) */
char BASE_URI[BUFSIZE] = "";

/* default directory to place indices */
char DEFAULT_INDEX[BUFSIZE] = OPT_INDEXDIR;

/* default directory to place config files */
char CONFDIR[BUFSIZE] = OPT_CONFDIR;

/* namazurc */
char NAMAZURC[BUFSIZE] = "";

#ifdef OPT_SCORING
#define TFIDF  1
#define SIMPLE 0
int TfIdf = OPT_SCORING;   /* switch of TfIdf mode */
#else
int TfIdf = 1;
#endif

struct nmz_replace *Replace = NULL;
struct nmz_alias   *Alias   = NULL;

struct nmz_files Nmz;  /* NMZ.* files' file pointers */
struct nmz_names NMZ = {  /* NMZ.* files' names */
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

struct nmz_indices Idx;
struct nmz_query Query;

