/*
 * l10n-ja.c -
 * $Id: l10n-ja.c,v 1.1 2000-02-13 17:34:53 rug Exp $
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

/*
 *
 * Public functions
 *
 */
#include <string.h>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "i18n.h"
#include "l10n-ja.h"

/* 
 * FIXME: ad-hoc function. 
 * Currently, it does't work fine if LANG is set "de_DE:ja_JP:C:en".
 */

int 
nmz_is_lang_ja(void)
{
    const char *lang;

    lang = nmz_get_lang();
    if (strcmp(lang, "japanese")) {
	return 1; /* TRUE */
    } 
    if (strcmp(lang, "ja")) {
	return 1; /* TRUE */
    } 
    if (strncmp(lang, "ja_JP", 5)) {
	return 1; /* TRUE */
    } 
    return 0; /* FALSE */
}
