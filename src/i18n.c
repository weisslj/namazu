/*
 * i18n.c -
 * $Id: i18n.c,v 1.2 1999-10-11 04:25:25 satoru Exp $
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
#include <stdlib.h>
#include <string.h>
#include "namazu.h"
#include "util.h"
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif

static char Lang[BUFSIZE] = "C";

/************************************************************
 *
 * Public functions
 *
 ************************************************************/

int is_lang_ja(void)
{
    if (strcmp(Lang, "japanese")) {
	return 1;
    } 
    if (strcmp(Lang, "ja")) {
	return 1;
    } 
    if (strprefixcmp(Lang, "ja_JP")) {
	return 1;
    } 
    return 0;
}

char *set_lang(char *lang)
{
    if (*lang != '\0') {  /* given lang is explicitly specified */
	strcpy(Lang, lang);
    } else {
	if ((lang = getenv("LC_ALL")) != NULL) {
	    strcpy(Lang, lang);
	} else if ((lang = getenv("LC_MESSAGES")) != NULL) {
	    strcpy(Lang, lang);
	} else if ((lang = getenv("LANG")) != NULL) {
	    strcpy(Lang, lang);
	} else {
	    strcpy(Lang, "C");
	}
    } 

#if HAVE_SETENV || HAVE_PUTENV
# if HAVE_SETENV
    setenv("LC_ALL", Lang, 1);
    setenv("LC_MESSAGES", Lang, 1);
    setenv("LANG", Lang, 1);
# else
    {
	uchar *buf;

	buf = malloc(BUFSIZE);
	sprintf(buf, "LC_ALL=%s", Lang);
	putenv(buf);

	buf = malloc(BUFSIZE);
	sprintf(buf, "LC_MESSAGES=%s", Lang);
	putenv(buf);

	buf = malloc(BUFSIZE);
	sprintf(buf, "LANG=%s", Lang);
	putenv(buf);

    }
# endif
#endif

    /* set enviromental variables for gettext() */
#ifdef HAVE_SETLOCALE
    /* Set locale via LC_ALL.  */
    setlocale (LC_ALL, "");
#endif

    return Lang;
}

char *get_lang(void) 
{
    return Lang;
}

/*
 * Choose message filename such as NMZ.tips.ja_JP.iso-2022-jp 
 *
 * NOTES: This function overwrite given `fname', so you
 * should pass `fname' as a temporary variable. Don't pass a
 * critical variable directly.  
 */
uchar *choose_msgfile(uchar *fname)
{
    FILE *fp;
    int base_leng;

    base_leng = strlen(fname);
    strcat(fname, ".");
    strcat(fname, get_lang());

    /* 
     * Trial example:
     * 1. NMZ.tips.ja_JP.iso-2022-jp
     * 2. NMZ.tips.ja_JP
     * 3. NMZ.tips.ja
     * 4. NMZ.tips
     */

    do {
	int i;

	fp = fopen(fname, "rb");
	if (fp != NULL) { /* fopen success */
	    debug_printf("choose_msgfile: %s open SUCCESS.\n", fname);
	    fclose(fp);
	    return fname;
	}
	debug_printf("choose_msgfile: %s open failed.\n", fname);

	for (i = strlen(fname) - 1;  i >= 0; i--) {
	    if (fname[i] == '.' ||
		fname[i] == '_')
	    {
		fname[i] = '\0';
		break;
	    }
	}
	if (strlen(fname) < base_leng) {
	    break;
	}
    } while (1);
    return NULL;
}


