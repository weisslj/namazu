/*
 * 
 * wakati.c -
 * 
 * $Id: wakati.c,v 1.33 2007-12-05 15:52:20 opengl2772 Exp $
 * 
 * Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.
 * Copyright (C) 2000,2007 Namazu Project All rights reserved.
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

#include <stdio.h>
#include <ctype.h>

#ifdef HAVE_STRING_H
#  include <string.h>
#else
#  include <strings.h>
#endif

#include "libnamazu.h"
#include "util.h"
#include "search.h"
#include "wakati.h"

/*
 *
 * Macros
 *
 */

#define iskanji(c)  (nmz_iseuc(*(c)) && nmz_iseuc(*(c + 1)))
#define iskanji_utf8(c)  (nmz_isutf8_3byte_1st(*(c)) && nmz_isutf8_multibyte_2nd(*(c + 1)) && nmz_isutf8_multibyte_2nd(*(c + 2)))


/*
 *
 * Private functions
 *
 */

static int detect_char_type(char *c);
static int iskatakana_utf8(const char *chr);
static int ishiragana_utf8(const char *chr);

static int 
detect_char_type(char *c)
{
    if (iskatakana_utf8(c)) {
        return KATAKANA;
    } else if (ishiragana_utf8(c)){
        return HIRAGANA;
    } else if (iskanji_utf8(c)) {
        return KANJI;
    }
    return OTHER;
}

static int 
iskatakana_utf8(const char *chr)
{
    uchar *c;
    c = (uchar *)chr;

    if (*c == 0xe3) {
	if (((*(c + 1) == 0x82) && 
	   ((*(c + 2) >= 0xa1) && (*(c + 2) <= 0xbf))) || 
	   ((*(c + 1) == 0x83) && 
	   ((*(c + 2) >= 0x80) && (*(c + 2) <= 0xb6)))) 
	{
	    return 1;
	} else if ((*(c + 1) == 0x83) && (*(c + 2) == 0xbc)) { /* choon */ 
            return 1;
	}
    } else {
	;
    }

    return 0;
}

static int 
ishiragana_utf8(const char *chr)
{
    uchar *c;
    c = (uchar *)chr;

    if (*c == 0xe3) {
	if (((*(c + 1) == 0x81) && 
	   ((*(c + 2) >= 0x81) && (*(c + 2) <= 0xbf))) || 
	   ((*(c + 1) == 0x82) && 
	   ((*(c + 2) >= 0x80) && (*(c + 2) <= 0x93)))) 
	{
	    return 1;
	} else if ((*(c + 1) == 0x83) && (*(c + 2) == 0xbc)) { /* choon */ 
            return 1;
	}
    } else {
	;
    }

    return 0;
}


/*
 *
 * Public functions
 *
 */

int 
nmz_wakati(char *key)
{
    int i, j, key_leng, type;
    char buf[BUFSIZE * 2] = "";

    nmz_debug_printf("wakati original: [%s].\n", key);

    for (i = 0; i < (int)strlen(key); ) {
        type = detect_char_type(key + i);
	if (nmz_isutf8_3byte_1st(*(key + i))) {
	    key_leng = 0;
	    for (j = 0; iskanji_utf8(key + i + j) ;  j += 3) {
		char tmp[BUFSIZE];

                if (j == 0 && (iskatakana_utf8(key + i + j) ||
                    ishiragana_utf8(key + i + j))) 
                {
                    /* If beggining character is Katakana or Hiragana */
                    break;
                }

		strncpy(tmp, key + i, j + 3);
		*(tmp + j + 3) = '\0';

		if (nmz_binsearch(tmp, 0) != -1) {
		    key_leng = j + 3;
		}
	    }

	    if (key_leng > 0) {
		strncat(buf, key + i, key_leng); 
                strcat(buf, "\t");
		i += key_leng;
	    } else {
                if (type == HIRAGANA || type == KATAKANA) {
                    for (j =0; ; j += 3) {
                        if (!((type == HIRAGANA && ishiragana_utf8(key + i + j))
                            ||(type == KATAKANA && iskatakana_utf8(key + i + j)))) 
                        {
                            break;
                        }
                        strncat(buf, key + i + j, 3);
                    }
                    i += j;
                    strcat(buf, "\t");
                } else {
                    strncat(buf, key + i, 3);
                    strcat(buf, "\t");
                    i += 3;
                }
	    }
	} else {
            while(*(key + i) && !nmz_isutf8_3byte_1st(*(key + i))) {
                /* As an initial attempt always success, 
                   outer 'for loop' can avoid infinite loop */
                if (*(key + i) == '\t') {
                    nmz_chomp(buf);
                }
                strncat(buf, key + i, 1);
                i++;
            }
            nmz_chomp(buf);
            strcat(buf, "\t");
	}
    }
    nmz_chomp(buf);

    if (strlen(buf) <= BUFSIZE) {
	strcpy(key, buf);
    } else {
	nmz_set_dyingmsg(nmz_msg("wakatigaki processing failed.\n"));
	return 1; 
    }
    nmz_debug_printf("wakatied string: [%s]\n", key);
    return 0;
}


