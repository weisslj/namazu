/*
 * 
 * re.c -
 * 
 * $Id: re.c,v 1.28 2000-01-28 09:40:12 satoru Exp $
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
#include "regex.h"
#include "util.h"
#include "hlist.h"
#include "replace.h"
#include "re.h"
#include "i18n.h"
#include "var.h"

#define STEP 256

/*
 *
 * Public functions
 *
 */

/*
 * FIXME: Dirty coding...
 */
NmzResult 
nmz_regex_grep(const char *expr, FILE *fp, const char *field, int field_mode)
{
    char buf[BUFSIZE], tmpexpr[BUFSIZE];
    struct re_pattern_buffer *rp;
    int i, n, size = 0, max, uri_mode = 0;
    NmzResult val, tmp;
    val.num = 0;

    if (nmz_is_lang_ja()) {
        nmz_re_mbcinit(MBCTYPE_EUC);
    } else {
        nmz_re_mbcinit(MBCTYPE_ASCII);
    }
    rp = ALLOC(struct re_pattern_buffer);
    MEMZERO((char *)rp, struct re_pattern_buffer, 1);
    rp->buffer = 0;
    rp->allocated = 0;
    
    strcpy(tmpexpr, expr); /* save orig_expr */
    nmz_debug_printf("REGEX: '%s'\n", tmpexpr);

    if (field_mode) {
        nmz_malloc_hlist(&val, size += STEP);
	if (val.stat == ERR_FATAL)
	    return val;
	val.num = 0; /* set 0 for no matching case */
        max = IGNORE_HIT;
        if (strcmp(field, "uri") == 0) {
            uri_mode = 1;
        }
    } else {
        max = IGNORE_MATCH;
    }

    nmz_re_compile_pattern(tmpexpr, strlen(tmpexpr), rp);

    for (i = n = 0; fgets(buf, BUFSIZE, fp); i++) {
        if (buf[strlen(buf) - 1] != '\n') {  /* too long */
            i--;
            continue;
        }
        buf[strlen(buf) - 1] = '\0';  /* LF to NULL */
        if (strlen(buf) == 0) {
            continue;
        }
        if (uri_mode) {  /* consider the REPLACE directive in namazurc */ 
            nmz_replace_uri(buf);
        }
        nmz_strlower(buf);
        if (-1 != nmz_re_search(rp, buf, strlen(buf), 0, strlen(buf), 0)) { 
           /* Matched */
            n++;
            if (n > max) {
                nmz_free_hlist(val);
                val.num = 0;
		val.stat = ERR_TOO_MUCH_MATCH;
                break;
            }
            if (!field_mode) {
                tmp = nmz_get_hlist(i);
		if (tmp.stat == ERR_FATAL)
		    return tmp;
                if (tmp.num > IGNORE_HIT) {
                    nmz_free_hlist(val);
                    val.stat = ERR_TOO_MUCH_HIT;
                    val.num = 0;
                    break;
                }
            } else {
                if (n > size) {
                    nmz_realloc_hlist(&val, size += STEP);
		    if (val.stat == ERR_FATAL)
		        return val;
                }
                val.data[n-1].docid = i;
                val.data[n-1].score = 1;  /* score = 1 */
                val.num = n;
            }

            if (!field_mode) {
                val = nmz_ormerge(val, tmp);
		if (val.stat == ERR_FATAL)
		    return val;
            } 
            if (val.num > IGNORE_HIT) {
                nmz_free_hlist(val);
                val.num = -1;
                break;
            }
	    if (nmz_is_debugmode()) {
                char buf2[BUFSIZE];

                if (field_mode) {
                    nmz_debug_printf("field: [%d]<%s> id: %d\n", 
                            val.num, buf, i);
                } else {
                    fseek(Nmz.w, nmz_getidxptr(Nmz.wi, i), 0);
                    fgets(buf2, BUFSIZE, Nmz.w);
                    nmz_chomp(buf2);
                    nmz_debug_printf("re: %s, (%d:%s), %d, %d\n", 
                            buf2, i, buf, tmp.num, val.num);
                }
	    }
        }
    }
    if (field_mode) {
        val = nmz_do_date_processing(val);
    }

    nmz_re_free_pattern(rp);
    return val;
}



