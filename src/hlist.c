/*
 * 
 * hlist.c -
 * 
 * $Id: hlist.c,v 1.12 1999-09-01 01:11:23 satoru Exp $
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

static int DocNum = 0;  /* Number of documents covered in atarget index */

/************************************************************
 *
 * Private functions
 *
 ************************************************************/

void memcpy_hlist(HLIST, HLIST, int);
void set_date_zero_all(HLIST);

void memcpy_hlist(HLIST to, HLIST from, int n)
{
    memcpy(to.fid + n, from.fid, from.n * sizeof (int));
    memcpy(to.scr + n, from.scr, from.n * sizeof (int));
    memcpy(to.did + n, from.did, from.n * sizeof (int));
    memcpy(to.date + n, from.date, from.n * sizeof (int));
}

void set_date_zero_all(HLIST hlist)
{
    int i;

    for (i = 0; i < hlist.n; i++) {
        hlist.date[i] = 0;
    }
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
	    if (left.fid[i] < right.fid[j])
		break;
	    if (left.fid[i] == right.fid[j]) {

		copy_hlist(left, v, left, i);
                if (TfIdf) {
                    left.scr[v] = left.scr[i] + right.scr[j];
                } else {
                    /* assign a smaller number, left or right*/
                    left.scr[v] = left.scr[i] < right.scr[j] ?
                        left.scr[i] : right.scr[j];
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
	    if (left.fid[i] < right.fid[j])
		break;
	    if (left.fid[i] == right.fid[j]) {
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
	for (; left.fid[i] >= right.fid[j] && j < right.n; j++) {
	    if (left.fid[i] == right.fid[j]) {

                if (TfIdf) {
                    left.scr[i] = left.scr[i] + right.scr[j];
                } else {
                    /* assign a large number, left or right */
                    left.scr[i] = left.scr[i] > right.scr[j] ?
                        left.scr[i] : right.scr[j];
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
    hlist->fid = (int *)malloc(n * sizeof(int));
    if (hlist->fid == NULL) {
	 die("malloc_hlist");
    }
    hlist->scr = (int *)malloc(n * sizeof(int));
    if (hlist->scr == NULL) {
	 die("malloc_hlist");
    }
    hlist->did = (int *)malloc(n * sizeof(int));
    if (hlist->did == NULL) {
	 die("malloc_hlist");
    }
    hlist->date = (int *)malloc(n * sizeof(int));
    if (hlist->date == NULL) {
	 die("malloc_hlist");
    }
}

void realloc_hlist(HLIST * hlist, int n)
{
    if (n <= 0) return;
    hlist->fid = (int *) realloc(hlist->fid, n * sizeof(int));
    if (hlist->fid == NULL) {
	 die("realloc_hlist");
    }
    hlist->scr = (int *) realloc(hlist->scr, n * sizeof(int));
    if (hlist->scr == NULL) {
	 die("realloc_hlist");
    }
    hlist->did = (int *) realloc(hlist->did, n * sizeof(int));
    if (hlist->did == NULL) {
	 die("realloc_hlist");
    }
    hlist->date = (int *) realloc(hlist->date, n * sizeof(int));
    if (hlist->date == NULL) {
	 die("realloc_hlist");
    }
}

void free_hlist(HLIST hlist)
{
    if (hlist.n <= 0) return;
    free(hlist.fid);
    free(hlist.scr);
    free(hlist.did);
    free(hlist.date);
}

void copy_hlist(HLIST to, int n_to, HLIST from, int n_from)
{
    to.fid[n_to] = from.fid[n_from];
    to.scr[n_to] = from.scr[n_from];
    to.did[n_to] = from.did[n_from];
    to.date[n_to] = from.date[n_from];
}

void set_did_hlist(HLIST hlist, int id)
{
    int i;
    for (i = 0; i < hlist.n; i++) {
        hlist.did[i] = id;
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
        set_date_zero_all(hlist);
        return hlist; /* error */
    }

    for (i = 0; i < hlist.n ; i++) {
        if (-1 == fseek(date_index, hlist.fid[i] * sizeof(hlist.date[i]), 0)) {
            set_date_zero_all(hlist);
            return hlist; /* error */
        }
        freadx(&hlist.date[i], sizeof(hlist.date[i]), 1, date_index);

        if (hlist.date[i] == -1) {  
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
    uchar tmp[BUFSIZE];
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
	    hlist.fid[i / 2] = *(buf + i) + sum;
	    sum = hlist.fid[i / 2];
	    hlist.scr[i / 2] = *(buf + i + 1);
            if (TfIdf) {
                hlist.scr[i / 2] = (int)(hlist.scr[i / 2] * idf) + 1;
            }
	}
        hlist.n = n / 2;
	free(buf);
        hlist = do_date_processing(hlist);
    } 
    return hlist;
}


/* use merge sort a stable sort algorithm 
 * original of this code was contributed by Furukawa-san [1997-11-13]
 */

void nmz_mergesort(int first, int last, HLIST hlist, HLIST work, int mode)
{
    int middle;
    static int i, j, k, p;
    
    if (first < last) {
	middle = (first + last) / 2;
	nmz_mergesort(first, middle, hlist, work, mode);
	nmz_mergesort(middle + 1, last, hlist, work, mode);
	p = 0;
	for (i = first; i <= middle; i++) {
	    copy_hlist(work, p, hlist, i);
	    p++;
	}
	i = middle + 1;
	j = 0;
	k = first;
	while (i <= last && j < p) {
            int bool = 0;

            if (mode == SORT_BY_SCORE) {
                if (work.scr[j] >= hlist.scr[i]) {
                    bool = 1;
                }
            } else if (mode == SORT_BY_DATE){
                if (work.date[j] >= hlist.date[i]) {
                    bool = 1;
                }
            }
	    if (bool) {
		copy_hlist(hlist, k, work, j);
		k++;
		j++;
	    } else {
		copy_hlist(hlist, k, hlist, i);
		k++;
		i++;
	    }
	}
	while (j < p) {
	    copy_hlist(hlist, k, work, j);
	    k++;
	    j++;
	}
    }
}


/* interface to invoke merge sort function */
void sort_hlist(HLIST hlist, int mode)
{
    HLIST work;
    malloc_hlist(&work, hlist.n);

    nmz_mergesort(0, hlist.n - 1, hlist, work, mode);
    free_hlist(work);
}

/* 
 * reverse the hlist
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

