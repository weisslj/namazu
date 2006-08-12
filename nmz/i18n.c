/*
 * i18n.c -
 * $Id: i18n.c,v 1.34 2006-08-12 07:01:01 opengl2772 Exp $
 * 
 * Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.
 * Copyright (C) 2000,2001 Namazu Project All rights reserved.
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
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif

#include "libnamazu.h"
#include "util.h"
#include "i18n.h"

/*
 *
 * Private functions
 *
 */

static const char *guess_category_value ( const char *categoryname );
static char *get_lang_by_category ( const char *categoryname );


/* The following is (partly) taken from the gettext package.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.  */

static const char *
guess_category_value (const char *categoryname)
{
    const char *retval;

    if (!strcmp(categoryname, "LC_MESSAGES")) {

	/* The highest priority value is the `LANGUAGE' environment
	   variable.  This is a GNU extension.  */
	retval = getenv ("LANGUAGE");
	if (retval != NULL && retval[0] != '\0')
	    return retval;
    }

    /* `LANGUAGE' is not set.  So we have to proceed with the POSIX
       methods of looking to `LC_ALL', `LC_xxx', and `LANG'.  On some
       systems this can be done by the `setlocale' function itself.  */

    /* Setting of LC_ALL overwrites all other.  */
    retval = getenv ("LC_ALL");  
    if (retval != NULL && retval[0] != '\0')
	return retval;

    /* Next comes the name of the desired category.  */
    retval = getenv (categoryname);
    if (retval != NULL && retval[0] != '\0')
	return retval;

    /* Last possibility is the LANG environment variable.  */
    retval = getenv ("LANG");
    if (retval != NULL && retval[0] != '\0')
	return retval;

    return NULL;
}

static char *
get_lang_by_category(const char *categoryname) 
{
    char *lang;
    lang = (char *)guess_category_value(categoryname);
    if (lang == NULL)
	return "C";
    else
	return lang;
}

/*
 *
 * Public functions
 *
 */

char *
nmz_set_lang(const char *lang)
{
    const char* env;

    env = guess_category_value("LC_MESSAGES");
    if (env == NULL && *lang != '\0') {
#ifdef HAVE_SETENV
	setenv("LANG", lang, 1);
#else
# ifdef HAVE_PUTENV
	{
	    static char *store = NULL;

	    if (store != NULL)
		free(store);
	    store = (char *)malloc(strlen(lang) + 6); /* do *not* free */
	    if (store == NULL)
		return NULL; /* FIXME: should be fatal error */
	    strcpy(store, "LANG=");
	    strcat(store, lang);
	    putenv(store);
	}
# endif
#endif
    }

#ifdef HAVE_SETLOCALE
    /* Set locale via LC_ALL.  */
    setlocale (LC_ALL, "");
#endif

    return (char *)lang;
}

char *
nmz_get_lang(void)
{
    return get_lang_by_category("LC_MESSAGES");
}

char *
nmz_get_lang_ctype(void)
{
    return get_lang_by_category("LC_CTYPE");
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
    char fname[BUFSIZE] = "";

    strncpy(fname, pfname, BUFSIZE - 1);
    baselen = strlen(fname);
    strncat(fname, ".", BUFSIZE - strlen(fname) - 1);
    strncat(fname, nmz_get_lang(), BUFSIZE - strlen(fname) - 1);

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
	    strcpy(lang_suffix, fname + baselen); /* it shouldn't be flood  */
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


