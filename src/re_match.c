/*
 * 
 * re_match.c -
 * 
 * $Id: re_match.c,v 1.3 1999-05-14 04:38:51 satoru Exp $
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
#include <stdlib.h>
#include <string.h>
#include "regex.h"
#include "namazu.h"
#include "util.h"

#define ALLOC_N(type,n) (type*)xmalloc(sizeof(type)*(n))
#define ALLOC(type) (type*)xmalloc(sizeof(type))
#define MEMZERO(p,type,n) memset((p), 0, sizeof(type)*(n))
typedef struct re_pattern_buffer Regexp;

#define STEP 256

HLIST regex_grep(uchar *orig_expr, FILE *fp, uchar *field, int field_mode)
{
    unsigned char buf[BUFSIZE], expr[BUFSIZE];
    struct re_pattern_buffer *rp;
    int i, n, size = 0, max, url_mode = 0;
    HLIST val, tmp;
    val.n = 0;

    if (is_lang_ja(Lang)) {
        re_mbcinit(MBCTYPE_EUC);
    } else {
        re_mbcinit(MBCTYPE_ASCII);
    }
    rp = ALLOC(Regexp);
    MEMZERO((char *)rp, Regexp, 1);
    rp->buffer = ALLOC_N(char, 16);
    rp->allocated = 16;
    rp->fastmap = ALLOC_N(char, 256);

    
    strcpy(expr, orig_expr); /* save orig_expr */
    if (Debug)
        fprintf(stderr, "REGEX EXPRESSION is '%s'\n", expr);


    if (field_mode) {
        malloc_hlist(&val, size += STEP);
        max = IGNORE_HIT;
        if (!strcmp(field, "url")) {
            url_mode = 1;
        }
    } else {
        max = IGNORE_MATCH;
    }

    re_compile_pattern(expr, strlen(expr), rp);

    for (i = n = 0; fgets(buf, BUFSIZE, fp); i++) {
        if (*(lastc(buf)) != '\n') {  /* too long */
            i--;
            continue;
        }
        *(lastc(buf)) = '\0';  /* LF to NULL */
        if (strlen(buf) == 0) {
            continue;
        }
        if (url_mode) {  /* consider the REPLACE directive in namazu.conf */ 
            replace_url(buf, 0);
        }
        tolower_string(buf);
        if (-1 != re_search(rp, buf, strlen(buf), 0, strlen(buf), 0)) { 
           /* matched */
            n++;
            if (n > max) {
                free_hlist(val);
                val.n = -1;
                break;
            }
            if (!field_mode) {
                tmp = get_hlist(i);
                if (tmp.n > IGNORE_HIT) {
                    free_hlist(val);
                    val.n = -1;
                    break;
                }
            } else {
                if (n > size) {
                    realloc_hlist(&val, size += STEP);
                }
                val.fid[n - 1] = i;
                val.scr[n - 1] = 1;  /* score = 1 */
                val.n = n;
            }

            if (!field_mode) {
                val = ormerge(val, tmp);
            } 
            if (val.n > IGNORE_HIT) {
                free_hlist(val);
                val.n = -1;
                break;
            }
	    if (Debug) {
                uchar buf2[BUFSIZE];

                if (field_mode) {
                    fprintf(stderr, "field: [%d]<%s> id: %d\n", 
                            val.n, buf, i);
                } else {
                    fseek(Index, get_index_pointer(IndexIndex, i), 0);
                    fgets(buf2, BUFSIZE, Index); /* read and dispose */
                    chop(buf2);
                    fprintf(stderr, "re: %s, (%d:%s), %d, %d\n", 
                            buf2, i, buf, tmp.n, val.n);
                }
	    }
        }
    }
    if (field_mode) {
        val = do_date_processing(val);
    }

    free(rp->buffer);
    free(rp->fastmap);
    return val;
}



