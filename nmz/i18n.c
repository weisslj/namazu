/*
 * i18n.c -
 * $Id: i18n.c,v 1.19 2000-02-12 13:34:58 rug Exp $
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
#include <string.h>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "libnamazu.h"
#include "util.h"
#include "i18n.h"
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif

static char Lang[BUFSIZE] = "C";

/*
 *
 * Public functions
 *
 */

int 
nmz_is_lang_ja(void)
{
    if (strcmp(Lang, "japanese")) {
	return 1;
    } 
    if (strcmp(Lang, "ja")) {
	return 1;
    } 
    if (nmz_strprefixcmp(Lang, "ja_JP")) {
	return 1;
    } 
    return 0;
}

char *
nmz_set_lang(const char *lang)
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
	    /* No locale is set as an envornment variable. */
	    return Lang;
	}
    } 

    /* 
     * Set enviromental variables for gettext() only if LANG is not set.
     */
    if (getenv("LANG") == NULL) {
	setenv("LC_ALL", Lang, 1);
	setenv("LC_MESSAGES", Lang, 1);
	setenv("LANG", Lang, 1);
    }

#ifdef HAVE_SETLOCALE
    /* Set locale via LC_ALL.  */
    setlocale (LC_ALL, "");
#endif

    return Lang;
}

char *
nmz_get_lang(void) 
{
    return Lang;
}

/*
 * Choose suffix of message files such as ".ja_JP", ".ja", "", etc.
 * lang
 */
enum nmz_stat
nmz_choose_msgfile_suffix(const char *pfname,  char *lang_suffix)
{
    FILE *fp;
    int baselen;
    char fname[BUFSIZE];

    strcpy(fname, pfname);
    baselen = strlen(fname);
    strcat(fname, ".");
    strcat(fname, nmz_get_lang());

    /* 
     * Trial example:
     * 1. NMZ.tips.ja_JP.ISO-2022-JP
     * 2. NMZ.tips.ja_JP
     * 3. NMZ.tips.ja
     * 4. NMZ.tips
     */

    do {
	int i;

	fp = fopen(fname, "rb");
	if (fp != NULL) { /* fopen success */
	    nmz_debug_printf("choose_msgfile: %s open SUCCESS.\n", fname);
	    fclose(fp);
	    strcpy(lang_suffix, fname + baselen);
	    return SUCCESS;
	}
	nmz_debug_printf("choose_msgfile: %s open failed.\n", fname);

	for (i = strlen(fname) - 1;  i >= 0; i--) {
	    if (fname[i] == '.' ||
		fname[i] == '_')
	    {
		fname[i] = '\0';
		break;
	    }
	}
	if (strlen(fname) < baselen) {
	    break;
	}
    } while (1);
    return FAILURE;
}


