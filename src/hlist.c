/*
 * 
 * hlist.c -
 * 
 * $Id: hlist.c,v 1.20 1999-09-03 10:20:54 satoru Exp $
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
 * This file must be encoded in EUC-JP encoding
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "namazu.h"
#include "util.h"
#include "hlist.h"
#include "field.h"

static int DocNum = 0;  /* Number of documents covered in atarget index */
static uchar Field[BUFSIZE] = "";  /* Field name used with sorting */

struct str_num {
    uchar *str;
    int   num;
};
typedef struct str_num str_num;

/************************************************************
 *
 * Private functions
 *
 ************************************************************/

void memcpy_hlist(HLIST, HLIST, int);
void init_date(HLIST);
void set_rank(HLIST);
void field_sort(HLIST);
int  field_scmp(const void*, const void*);
int  field_ncmp(const void*, const void*);
int  date_cmp(const void*, const void*);
int  score_cmp(const void*, const void*);

void memcpy_hlist(HLIST to, HLIST from, int n)
{
    memcpy(to.d + n,  from.d,  from.n * sizeof (to.d[0]));
}

void init_date(HLIST hlist)
{
    int i;

    for (i = 0; i < hlist.n; i++) {
        hlist.d[i].date = 0;
    }
}

void set_rank(HLIST hlist)
{
    int i;

    /* set rankings in descending order */
    for (i = 0 ; i < hlist.n; i++) {
        hlist.d[i].rank = hlist.n - i;
    }
}

void field_sort(HLIST hlist) 
{
    int i, numeric = 1;

    for (i = 0; i < hlist.n; i++) {
	uchar buf[BUFSIZE];
	int leng;
	get_field_data(hlist.d[i].did, hlist.d[i].fid, Field, buf);
	chomp(buf);
	leng = strlen(buf);

	if (numeric == 1 && ! isnumstr(buf)) {
	    numeric = 0;
	}

	hlist.d[i].field = (uchar *)malloc(leng + 1);
	if (hlist.d[i].field == NULL) {
	    die("void_field_sort");
	}
	strcpy(hlist.d[i].field, buf);
    } 

    if (numeric == 1) {
	qsort(hlist.d, hlist.n, sizeof(hlist.d[0]), field_ncmp);
	for (i = 0; i < hlist.n; i++) {
	    free(hlist.d[i].field);
	}
	
    } else {
	qsort(hlist.d, hlist.n, sizeof(hlist.d[0]), field_scmp);
	for (i = 0; i < hlist.n; i++) {
	    free(hlist.d[i].field);
	}
    }
}

/* field_scmp: 
   compare of a pair of hlist.d[].field as string in descending order */
int field_scmp(const void *p1, const void *p2)
{
    hlist_data *v1, *v2;
    int r;

    v1 = (hlist_data *) p1;
    v2 = (hlist_data *) p2;

    r = strcmp(v2->field, v1->field);
    if (r == 0) {
        /* NOTE: comparison "a - b" is not safe for NEGATIVE numbers */
	r = v2->rank - v1->rank;
    }
    return r;
}

/* field_ncmp: 
   compare of a pair of hlist.d[].field as number in descending order */
int field_ncmp(const void *p1, const void *p2)
{
    hlist_data *v1, *v2;
    int r;

    v1 = (hlist_data *) p1;
    v2 = (hlist_data *) p2;

    /* NOTE: comparison "a - b" is not safe for NEGATIVE numbers */
    r = atoi(v2->field) - atoi(v1->field);
    if (r ==0) {
	r = v2->rank -v1->rank;
    }
    return r;
}


/* score_ncmp: 
   compare of a pair of hlist.d[].score as number in descending order */
int score_cmp(const void *p1, const void *p2)
{
    hlist_data *v1, *v2;
    int r;
    
    v1 = (hlist_data *) p1;
    v2 = (hlist_data *) p2;

    /* NOTE: comparison "a - b" is not safe for NEGATIVE numbers */
    r = v2->scr - v1->scr;
    if (r == 0) {
	r = v2->rank - v1->rank;
    }
    return r;
    /* return (r = v2->scr - v1->scr) ? r : v2->rank - v1->rank; */
}

/* date_ncmp: 
   compare of a pair of hlist.d[].date as number in descending order */
int date_cmp(const void *p1, const void *p2)
{
    hlist_data *v1, *v2;
    int r;

    v1 = (hlist_data *) p1;
    v2 = (hlist_data *) p2;

    /* NOTE: comparison "a - b" is not safe for NEGATIVE numbers */
    r = v2->date - v1->date;
    if (r == 0) {
	r = v2->rank - v1->rank;
    } 
    return r;
}

/************************************************************
 *
 * Public functions
 *
 ************************************************************/

/* merge the left and  right with AND rule */
HLIST andmerge(HLIST left, HLIST right, int *ignore)
{
    int i, j, v;

    if (*ignore && left.n > 0)
	return left;
    if (*ignore && right.n > 0)
	return right;

    if (left.n <= 0)
	return left;
    if (right.n <= 0)
	return right;

    for (v = 0, i = 0, j = 0; i < left.n; i++) {
	for (;; j++) {
	    if (j >= right.n)
		goto OUT;
	    if (left.d[i].fid < right.d[j].fid)
		break;
	    if (left.d[i].fid == right.d[j].fid) {

		copy_hlist(left, v, left, i);
                if (TfIdf) {
                    left.d[v].scr = left.d[i].scr + right.d[j].scr;
                } else {
                    /* assign a smaller number, left or right*/
                    left.d[v].scr = left.d[i].scr < right.d[j].scr ?
                        left.d[i].scr : right.d[j].scr;
                }
		v++;
		j++;
		break;
	    }
	}
    }
  OUT:
    free_hlist(right);
    left.n = v;
    if (left.n <= 0)
	free_hlist(left);
    return left;
}


/* merge the left and  right with NOT rule */
HLIST notmerge(HLIST left, HLIST right, int *ignore)
{
    int i, j, v, f;

    if (*ignore && left.n > 0)
	return left;
    if (*ignore && right.n > 0)
	return right;

    if (right.n <= 0)
	return left;
    if (left.n <= 0)
	return left;

    for (v = 0, i = 0, j = 0; i < left.n; i++) {
	for (f = 0; j < right.n; j++) {
	    if (left.d[i].fid < right.d[j].fid)
		break;
	    if (left.d[i].fid == right.d[j].fid) {
		j++;
		f = 1;
		break;
	    }
	}
	if (!f) {
	    copy_hlist(left, v, left, i);
	    v++;
	}
    }
    free_hlist(right);
    left.n = v;
    if (left.n <= 0)
	free_hlist(left);
    return left;
}


/*
 * merge the left and  right with OR rule
 */
HLIST ormerge(HLIST left, HLIST right)
{
    int i, j, v, n;
    HLIST val;

    if (left.n <= 0 && right.n <= 0)
	return left;
    if (left.n <= 0)
	return right;
    if (right.n <= 0)
	return left;

    n = left.n + right.n;

    malloc_hlist(&val, n);

    for (v = 0, i = 0, j = 0; i < left.n; i++) {
	for (; left.d[i].fid >= right.d[j].fid && j < right.n; j++) {
	    if (left.d[i].fid == right.d[j].fid) {

                if (TfIdf) {
                    left.d[i].scr = left.d[i].scr + right.d[j].scr;
                } else {
                    /* assign a large number, left or right */
                    left.d[i].scr = left.d[i].scr > right.d[j].scr ?
                        left.d[i].scr : right.d[j].scr;
                }
		j++;
		break;
	    } else {
		copy_hlist(val, v, right, j);
		v++;
	    }
	}
	copy_hlist(val, v, left, i);
	v++;
    }

    for (; j < right.n; j++) {
	copy_hlist(val, v, right, j);
	v++;
    }

    free_hlist(left);
    free_hlist(right);
    val.n = v;
    return val;
}

void malloc_hlist(HLIST * hlist, int n)
{
    if (n <= 0) return;
    hlist->d = (hlist_data *)malloc(n * sizeof(hlist_data));
    if (hlist->d == NULL) {
	 die("malloc_hlist");
    }

    hlist->n = n;
}

void realloc_hlist(HLIST * hlist, int n)
{
    if (n <= 0) return;
    hlist->d = (hlist_data *) realloc(hlist->d, n * sizeof(hlist_data));
    if (hlist->d == NULL) {
	 die("realloc_hlist");
    }
}

void free_hlist(HLIST hlist)
{
    if (hlist.n <= 0) return;
    free(hlist.d);
}

void copy_hlist(HLIST to, int n_to, HLIST from, int n_from)
{
    to.d[n_to] = from.d[n_from];
}

void set_did_hlist(HLIST hlist, int id)
{
    int i;
    for (i = 0; i < hlist.n; i++) {
        hlist.d[i].did = id;
    }
}

HLIST merge_hlist(HLIST *hlists)
{
    int i, n;
    HLIST value;

    if (Idx.num == 1) return hlists[0];
    for(i = n = 0; i < Idx.num; i++) {
        if (hlists[i].n > 0) {
            n += hlists[i].n;
        }
    }
    malloc_hlist(&value, n);
    for(i = n = 0; i < Idx.num; i++) {
        if (hlists[i].n <= 0) 
            continue;
        memcpy_hlist(value, hlists[i], n);
        n += hlists[i].n;
        free_hlist(hlists[i]);
    }
    value.n = n;
    return value;
}

/* get date info from NMZ.t and do the missing number processing */
HLIST do_date_processing(HLIST hlist)
{
    FILE *date_index;
    int i;

    date_index = fopen(NMZ.t, "rb");
    if (date_index == NULL) {
        if (Debug) {
            fprintf(stderr, "%s: cannot open file.\n", NMZ.t);
        }
        init_date(hlist);
        return hlist; /* error */
    }

    for (i = 0; i < hlist.n ; i++) {
        if (-1 == fseek(date_index, hlist.d[i].fid * sizeof(hlist.d[i].date), 0)) {
            init_date(hlist);
            return hlist; /* error */
        }
        freadx(&hlist.d[i].date, sizeof(hlist.d[i].date), 1, date_index);

        if (hlist.d[i].date == -1) {  
            /* the missing number, this document has been deleted */
            int j;

            for (j = i + 1; j < hlist.n; j++) { /* shift */
                copy_hlist(hlist, j - 1, hlist, j);
            }
            hlist.n--;
            i--;
        }
    }

    fclose(date_index);
    return hlist;
}

/* get the hit list */
HLIST get_hlist(int index)
{
    int n, *buf, i;
    HLIST hlist;
    double idf = 0;
    hlist.n = 0;

    if (-1 == fseek(Nmz.i, getidxptr(Nmz.ii, index), 0))
	return hlist; /* error */

    get_unpackw(Nmz.i, &n);

    if (TfIdf) {
        idf = log((double)DocNum / (n/2)) / log(2);
        if (Debug)
            fprintf(stderr, "idf: %f (N:%d, n:%d)\n", idf, DocNum, n/2);
    }

    if (n >= IGNORE_HIT * 2) {  
        /* '* 2' means NMZ.i contains a file-ID and a score. */
        hlist.n = ERR_TOO_MUCH_HIT;
    } else {
	int sum = 0;
	buf = (int *) malloc(n * sizeof(int)); /* with pelnty margin */
	if (buf == NULL) {
	    die("get_hlist");
	}
	n = read_unpackw(Nmz.i, buf, n);
	malloc_hlist(&hlist, n / 2);
	
	for (i = 0; i < n; i += 2) {
	    hlist.d[i/2].fid = *(buf + i) + sum;
	    sum = hlist.d[i/2].fid;
	    hlist.d[i/2].scr = *(buf + i + 1);
            if (TfIdf) {
                hlist.d[i/2].scr = (int)(hlist.d[i/2].scr * idf) + 1;
            }
	}
        hlist.n = n / 2;
	free(buf);
        hlist = do_date_processing(hlist);
    } 
    return hlist;
}


/* interface to invoke merge sort function */
void sort_hlist(HLIST hlist, int mode)
{
    set_rank(hlist); /* conserve current order for STABLE sorting */

    if (mode == SORT_BY_FIELD) {
	field_sort(hlist);
    } else if (mode == SORT_BY_DATE) {
	qsort(hlist.d, hlist.n, sizeof(hlist.d[0]), date_cmp);
    } else if (mode == SORT_BY_SCORE) {
	qsort(hlist.d, hlist.n, sizeof(hlist.d[0]), score_cmp);
    } 
}

/* 
 * reverse a given hlist
 * original of this routine was contributed by Furukawa-san [1997-11-13]
 */
void reverse_hlist(HLIST hlist)
{
    int m, n;
    HLIST tmp;

    malloc_hlist(&tmp, 1);
    m = 0;
    n = hlist.n - 1;
    while (m < n) {
	copy_hlist(tmp, 0, hlist, m);
	copy_hlist(hlist, m, hlist, n);
	copy_hlist(hlist, n, tmp, 0);
	m++;
	n--;
    }
    free_hlist(tmp);
}

void set_docnum(int n)
{
    DocNum = n;
}

void set_sort_field(uchar *field)
{
    strcpy(Field, field);
}

