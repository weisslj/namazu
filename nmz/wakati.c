/*
 * 
 * wakati.c -
 * 
 * $Id: wakati.c,v 1.22 2000-01-10 20:21:10 rug Exp $
 * 
 * Copyright (C) 1997-2000 Satoru Takabayashi  All rights reserved.
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
#include <string.h>
#include <ctype.h>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "libnamazu.h"
#include "message.h"
#include "util.h"
#include "field.h"
#include "codeconv.h"
#include "search.h"
#include "output.h"
#include "wakati.h"
#include "i18n.h"
#include "var.h"


/*
 *
 * Private functions
 *
 */

static int detect_char_type(char *c);

static int 
detect_char_type(char *c)
{
    if (nmz_iskatakana(c)) {
        return KATAKANA;
    } else if (nmz_ishiragana(c)){
        return HIRAGANA;
    } else if (nmz_iskanji(c)) {
        return KANJI;
    }
    return OTHER;
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

    for (i = 0; i < strlen(key); ) {
        type = detect_char_type(key + i);
	if (nmz_iseuc(*(key + i))) {
	    key_leng = 0;
	    for (j = 0; nmz_iskanji(key + i + j) ;  j += 2) {
		char tmp[BUFSIZE];

                if (j == 0 && (nmz_iskatakana(key + i + j) ||
                    nmz_ishiragana(key + i + j))) 
                {
                    /* If beggining character is Katakana or Hiragana */
                    break;
                }

		strncpy(tmp, key + i, j + 2);
		*(tmp + j + 2) = '\0';

		if (nmz_binsearch(tmp, 0) != -1) {
		    key_leng = j + 2;
		}
	    }

	    if (key_leng > 0) {
		strncat(buf, key + i, key_leng); 
                strcat(buf, "\t");
		i += key_leng;
	    } else {
                if (type == HIRAGANA || type == KATAKANA) {
                    for (j =0; ; j += 2) {
                        if (!((type == HIRAGANA && nmz_ishiragana(key + i + j))
                            ||(type == KATAKANA && nmz_iskatakana(key + i + j)))) 
                        {
                            break;
                        }
                        strncat(buf, key + i + j, 2);
                    }
                    i += j;
                    strcat(buf, "\t");
                } else {
                    strncat(buf, key + i, 2);
                    strcat(buf, "\t");
                    i += 2;
                }
	    }
	} else {
            while(*(key + i) && !nmz_iseuc(*(key + i))) {
                /* As an initial attempt always success, 
                   outer 'for loop' can avoid infinite loop */
                strncat(buf, key + i, 1);
                i++;
            }
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


