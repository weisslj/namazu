/*
 * 
 * re.c -
 * 
 * $Id: re.c,v 1.1 1999-11-08 05:06:05 knok Exp $
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
#include "regex.h"
#include "namazu.h"
#include "util.h"
#include "hlist.h"
#include "re.h"
#include "i18n.h"

#define STEP 256

/************************************************************
 *
 * Public functions
 *
 ************************************************************/

/* replace a URI */
int replace_uri(uchar *uri)
{
    int npat, nrep, i, j;
    uchar tmp[BUFSIZE];
    REPLACE *list = Replace;
    int is_regex_matching = 0;

    strcpy(tmp, uri);

    while (list) {
	struct re_registers regs;
	int mlen;
	REGEX *re;

	re = list->pat_re;
	regs.allocated = 0;

	if (re == NULL) {
	    /* Compiled regex is no available, we will apply the straight
	     * string substitution.
	     */
	    is_regex_matching = 0;
	} else if (0 < (mlen = re_match (re, tmp, strlen (tmp), 0, &regs))) {
	    /* We got a match.  Try to replace the string. */
	    uchar repl[BUFSIZE];
	    uchar *subst = list->rep;
	    /* Assume we are doing regexp match for now; if any of the
	     * substitution fails, we will switch back to the straight
	     * string substitution.
	     */
	    is_regex_matching = 1;
	    for (i = j = 0; subst[i]; i++) {
		/* i scans through RHS of sed-style substitution.
		 * j points at the string being built.
		 */
		if ((subst[i] == '\\') &&
		    ('0' <= subst[++i]) &&
		    (subst[i] <= '9')) 
		{
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
			is_regex_matching = 0;
			break;
		    }
		    for (ct = regs.beg[regno]; ct < regs.end[regno]; ct++)
			repl[j++] = tmp[ct];
		} else {
		    /* Either ordinary character, 
		     * or an unrecognized \ sequence.
		     * Just copy it.
		     */
		    repl[j++] = subst[i];
		}
	    }
	    if (is_regex_matching) {
		/* Good.  Regexp substitution worked and we now have a good
		 * string in repl.
		 * The part that matched and being replaced is 0 to mlen-1
		 * in tmp; tmp[mlen] through the end of it should be
		 * concatenated to the end of the resulting string.
		 */
		repl[j] = 0;
		strcpy(uri, repl);
		strcpy(uri + j, tmp + mlen);
	    }
	    re_free_registers (&regs);
	}
	if (is_regex_matching) {
	    return 0;
	}
	/* Otherwise, we fall back to string substitution */

	npat = strlen(list->pat);
	nrep = strlen(list->rep);

	if (strncmp(list->pat, tmp, npat) == 0) {
	    strcpy(uri, list->rep);
	    for (i = npat, j = nrep; tmp[i]; i++, j++) {
		uri[j] = tmp[i];
	    }
	    uri[j] = '\0';
	    return 1;
	}
	list = list->next;
    }
    return 0;
}

HLIST regex_grep(uchar *orig_expr, FILE *fp, uchar *field, int field_mode)
{
    unsigned char buf[BUFSIZE], expr[BUFSIZE];
    REGEX *rp;
    int i, n, size = 0, max, uri_mode = 0;
    HLIST val, tmp;
    val.n = 0;

    if (is_lang_ja()) {
        re_mbcinit(MBCTYPE_EUC);
    } else {
        re_mbcinit(MBCTYPE_ASCII);
    }
    rp = ALLOC(REGEX);
    MEMZERO((char *)rp, REGEX, 1);
    rp->buffer = 0;
    rp->allocated = 0;
    
    strcpy(expr, orig_expr); /* save orig_expr */
    debug_printf("REGEX: '%s'\n", expr);

    if (field_mode) {
        malloc_hlist(&val, size += STEP);
	if (val.n == DIE_HLIST)
	    return val;
	val.n = 0; /* set 0 for no matching case */
        max = IGNORE_HIT;
        if (strcmp(field, "uri") == 0) {
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
        if (uri_mode) {  /* consider the REPLACE directive in namazurc */ 
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
		if (tmp.n == DIE_HLIST)
		    return tmp;
                if (tmp.n > IGNORE_HIT) {
                    free_hlist(val);
                    val.n = -1;
                    break;
                }
            } else {
                if (n > size) {
                    realloc_hlist(&val, size += STEP);
		    if (val.n == DIE_HLIST)
		        return val;
                }
                val.d[n-1].docid = i;
                val.d[n-1].score = 1;  /* score = 1 */
                val.n = n;
            }

            if (!field_mode) {
                val = ormerge(val, tmp);
		if (val.n == DIE_HLIST)
		    return val;
            } 
            if (val.n > IGNORE_HIT) {
                free_hlist(val);
                val.n = -1;
                break;
            }
	    if (Debug) {
                uchar buf2[BUFSIZE];

                if (field_mode) {
                    debug_printf("field: [%d]<%s> id: %d\n", 
                            val.n, buf, i);
                } else {
                    fseek(Nmz.w, getidxptr(Nmz.wi, i), 0);
                    fgets(buf2, BUFSIZE, Nmz.w);
                    chomp(buf2);
                    debug_printf("re: %s, (%d:%s), %d, %d\n", 
                            buf2, i, buf, tmp.n, val.n);
                }
	    }
        }
    }
    if (field_mode) {
        val = do_date_processing(val);
    }

    re_free_pattern(rp);
    return val;
}



