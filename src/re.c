/*
 * 
 * re.c -
 * 
 * $Id: re.c,v 1.2 1999-08-27 03:55:26 satoru Exp $
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
#include "hlist.h"

#define ALLOC_N(type,n) (type*)xmalloc(sizeof(type)*(n))
#define ALLOC(type) (type*)xmalloc(sizeof(type))
#define MEMZERO(p,type,n) memset((p), 0, sizeof(type)*(n))
typedef struct re_pattern_buffer Regexp;

#define STEP 256

/************************************************************
 *
 * Public functions
 *
 ************************************************************/

/* replace a URI */
int replace_uri(uchar * s)
{
    int n_src, n_dst, i, j;
    uchar tmp[BUFSIZE];
    REPLACE list = Replace;

    strcpy(tmp, s);

    while (list.src) {
        if (strpbrk (list.src->str, ".*")) {
	    struct re_registers regs;
	    Regexp *re;
	    int mlen;
	    int is_a_regexp_match = 0;

	    regs.allocated = 0;
	    re = ALLOC(Regexp);
	    MEMZERO((char *)re, Regexp, 1);
	    re->buffer = 0;
	    re->allocated = 0;
	    if (re_compile_pattern (list.src->str, strlen (list.src->str), re))
	      /* re_comp fails; maybe it was not a regexp substitution
	       * after all.  Fall back to string substitution for backward
	       * compatibility.
	       */
	      is_a_regexp_match = 0;
	    else if (0 < (mlen = re_match (re, tmp, strlen (tmp), 0, &regs))) {
	      /* We got a match.  Try to replace the string. */
	      uchar repl[BUFSIZE];
	      uchar *subst = list.dst->str;
	      /* Assume we are doing regexp match for now; if any of the
	       * substitution fails, we will switch back to the straight
	       * string substitution.
	       */
	      is_a_regexp_match = 1;
	      for (i = j = 0; subst[i]; i++) {
		/* i scans through RHS of sed-style substitution.
		 * j points at the string being built.
		 */
		if ((subst[i] == '\\') &&
		    ('0' <= subst[++i]) &&
		    (subst[i] <= '9')) {
		  /* A backslash followed by a digit---regexp substitution.
		   * Note that a backslash followed by anything else is
		   * silently dropped (including a \\ sequence) and is
		   * passed on to the else clause.
		   */
		  int regno = subst[i] - '0';
		  int ct;
		  if (re->re_nsub <= regno) {
		    /* Oops; this is a bad substitution.  Just give up
		     * and use straight string substitution for backward
		     * compatibility.
		     */
		    is_a_regexp_match = 0;
		    break;
		  }
		  for (ct = regs.beg[regno]; ct < regs.end[regno]; ct++)
		    repl[j++] = tmp[ct];
		}
		else {
		  /* Either ordinary character, or an unrecognized \ sequence.
		   * Just copy it.
		   */
		  repl[j++] = subst[i];
		}
	      }
	      if (is_a_regexp_match) {
		/* Good.  Regexp substitution worked and we now have a good
		 * string in repl.
		 * The part that matched and being replaced is 0 to mlen-1
		 * in tmp; tmp[mlen] through the end of it should be
		 * concatenated to the end of the resulting string.
		 */
		repl[j] = 0;
		strcpy (s, repl);
		strcpy (s + j, tmp + mlen);
	      }
	      re_free_registers (&regs);
	    }
	    re_free_pattern (re);
	    if (is_a_regexp_match)
	      return 0;
	    /* Otherwise, we fall back to string substitution */
	}
	n_src = strlen(list.src->str);
	n_dst = strlen(list.dst->str);

	if (!strncmp(list.src->str, tmp, n_src)) {
	    strcpy(s, list.dst->str);
	    for (i = n_src, j = n_dst; tmp[i]; i++, j++) {
		s[j] = tmp[i];
	    }
	    s[j] = '\0';
	    return 1;
	}
	list.src = list.src->next;
	list.dst = list.dst->next;
    }
    return 0;
}

HLIST regex_grep(uchar *orig_expr, FILE *fp, uchar *field, int field_mode)
{
    unsigned char buf[BUFSIZE], expr[BUFSIZE];
    struct re_pattern_buffer *rp;
    int i, n, size = 0, max, uri_mode = 0;
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
        if (!strcmp(field, "uri")) {
            uri_mode = 1;
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
        if (uri_mode) {  /* consider the REPLACE directive in namazu.conf */ 
            replace_uri(buf);
        }
        strlower(buf);
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
                    fseek(Nmz.i, getidxptr(Nmz.ii, i), 0);
                    fgets(buf2, BUFSIZE, Nmz.i); /* read and dispose */
                    chomp(buf2);
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



