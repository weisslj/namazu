/*
 * 
 * hlist.c -
 * 
 * $Id: hlist.c,v 1.32 2000-01-08 09:27:19 satoru Exp $
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
 * This file must be encoded in EUC-JP encoding
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "libnamazu.h"
#include "util.h"
#include "hlist.h"
#include "field.h"
#include "var.h"
#include "idxname.h"

static int document_number = 0;  /* Number of documents covered in a target index */
static char field_for_sort[BUFSIZE] = "";  /* field_for_sort name used with sorting */

struct str_num {
    char *str;
    int   num;
};
typedef struct str_num str_num;

/*
 *
 * Private functions
 *
 */

static void memcpy_hlist(NmzResult, NmzResult, int);
static void set_rank(NmzResult);
static int  field_sort(NmzResult);
static int  field_scmp(const void*, const void*);
static int  field_ncmp(const void*, const void*);
static int  date_cmp(const void*, const void*);
static int  score_cmp(const void*, const void*);

static void 
memcpy_hlist(NmzResult to, NmzResult from, int n)
{
    memcpy(to.data + n,  from.data,  from.num * sizeof (to.data[0]));
}

static void 
set_rank(NmzResult hlist)
{
    int i;

    /* Set rankings in descending order */
    for (i = 0 ; i < hlist.num; i++) {
        hlist.data[i].rank = hlist.num - i;
    }
}

static int 
field_sort(NmzResult hlist) 
{
    int i, numeric = 1;

    for (i = 0; i < hlist.num; i++) {
	char buf[BUFSIZE];
	int leng;
	get_field_data(hlist.data[i].idxid, hlist.data[i].docid, field_for_sort, buf);
	nmz_chomp(buf);
	leng = strlen(buf);

	if (numeric == 1 && ! nmz_isnumstr(buf)) {
	    numeric = 0;
	}

	hlist.data[i].field = malloc(leng + 1);
	if (hlist.data[i].field == NULL) {
	    nmz_set_dyingmsg(nmz_msg("%s", strerror(errno)));
	    return FAILURE;
	}
	strcpy(hlist.data[i].field, buf);
    } 

    if (numeric == 1) {
	qsort(hlist.data, hlist.num, sizeof(hlist.data[0]), field_ncmp);
	
    } else {
	qsort(hlist.data, hlist.num, sizeof(hlist.data[0]), field_scmp);
    }

    for (i = 0; i < hlist.num; i++) {
	free(hlist.data[i].field);
    }
    return 0;
}

/* field_scmp: 
   compare of a pair of hlist.data[].field as string in descending order */
static int 
field_scmp(const void *p1, const void *p2)
{
    struct nmz_data *v1, *v2;
    int r;

    v1 = (struct nmz_data *) p1;
    v2 = (struct nmz_data *) p2;

    r = strcmp(v2->field, v1->field);
    if (r == 0) {
        /* NOTE: comparison "a - b" is not safe for NEGATIVE numbers */
	r = v2->rank - v1->rank;
    }
    return r;
}

/* field_ncmp: 
   compare of a pair of hlist.data[].field as number in descending order */
static int 
field_ncmp(const void *p1, const void *p2)
{
    struct nmz_data *v1, *v2;
    int r;

    v1 = (struct nmz_data *) p1;
    v2 = (struct nmz_data *) p2;

    /* NOTE: comparison "a - b" is not safe for NEGATIVE numbers */
    r = atoi(v2->field) - atoi(v1->field);
    if (r ==0) {
	r = v2->rank -v1->rank;
    }
    return r;
}


/* score_cmp: 
   compare of a pair of hlist.data[].score as number in descending order */
static int 
score_cmp(const void *p1, const void *p2)
{
    struct nmz_data *v1, *v2;
    int r;
    
    v1 = (struct nmz_data *) p1;
    v2 = (struct nmz_data *) p2;

    /* NOTE: comparison "a - b" is not safe for NEGATIVE numbers */
    r = v2->score - v1->score;
    if (r == 0) {
	r = v2->rank - v1->rank;
    }
    return r;
    /* Return (r = v2->score - v1->score) ? r : v2->rank - v1->rank; */
}

/* date_ncmp: 
   compare of a pair of hlist.data[].date as number in descending order */
static int 
date_cmp(const void *p1, const void *p2)
{
    struct nmz_data *v1, *v2;
    int r;

    v1 = (struct nmz_data *) p1;
    v2 = (struct nmz_data *) p2;

    /* NOTE: comparison "a - b" is not safe for NEGATIVE numbers */
    r = v2->date - v1->date;
    if (r == 0) {
	r = v2->rank - v1->rank;
    } 
    return r;
}

/*
 *
 * Public functions
 *
 */

/*
 * Merge the left and  right with AND rule
 */
NmzResult 
andmerge(NmzResult left, NmzResult right, int *ignore)
{
    int i, j, v;

    if (*ignore && left.num > 0) {
	free_hlist(right);
	return left;
    }
    if (*ignore && right.num > 0) {
	free_hlist(left);
	return right;
    }

    if (left.stat != SUCCESS || left.num <= 0) {
	free_hlist(right);
	return left;
    }
    if (right.stat != SUCCESS || right.num <= 0) {
	free_hlist(left);
	return right;
    }

    for (v = 0, i = 0, j = 0; i < left.num; i++) {
	for (;; j++) {
	    if (j >= right.num)
		goto OUT;
	    if (left.data[i].docid < right.data[j].docid)
		break;
	    if (left.data[i].docid == right.data[j].docid) {

		copy_hlist(left, v, left, i);
                if (TfIdf) {
                    left.data[v].score = left.data[i].score + right.data[j].score;
                } else {
                    /* Assign a smaller number, left or right*/
                    left.data[v].score = left.data[i].score < right.data[j].score ?
                        left.data[i].score : right.data[j].score;
                }
		v++;
		j++;
		break;
	    }
	}
    }
  OUT:
    free_hlist(right);
    left.num = v;
    if (left.stat != SUCCESS || left.num <= 0)
	free_hlist(left);
    return left;
}


/*
 * Merge the left and  right with NOT rule
 */
NmzResult 
notmerge(NmzResult left, NmzResult right, int *ignore)
{
    int i, j, v, f;

    if (*ignore && left.num > 0) {
	free_hlist(right);
	return left;
    }
    if (*ignore && right.num > 0) {
	free_hlist(left);
	return right;
    }

    if (right.stat != SUCCESS || right.num <= 0) {
	free_hlist(right);
	return left;
    }
    if (left.stat != SUCCESS || left.num <= 0) {
	free_hlist(left);
	return right;
    }

    for (v = 0, i = 0, j = 0; i < left.num; i++) {
	for (f = 0; j < right.num; j++) {
	    if (left.data[i].docid < right.data[j].docid)
		break;
	    if (left.data[i].docid == right.data[j].docid) {
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
    left.num = v;
    if (left.stat != SUCCESS || left.num <= 0)
	free_hlist(left);
    return left;
}


/*
 * merge the left and  right with OR rule
 */
NmzResult 
ormerge(NmzResult left, NmzResult right)
{
    int i, j, v, n;
    NmzResult val;

    if ((left.stat  != SUCCESS || left.num <= 0) && 
	(right.stat != SUCCESS || right.num <= 0)) 
    {
	free_hlist(right);
	return left;
    }
    if (left.stat != SUCCESS || left.num <= 0) {
	free_hlist(left);
	return right;
    }
    if (right.stat != SUCCESS || right.num <= 0){
	free_hlist(right);
	return left;
    }

    n = left.num + right.num;

    malloc_hlist(&val, n);
    if (val.stat == ERR_FATAL)
        return val;

    for (v = 0, i = 0, j = 0; i < left.num; i++) {
	for (; left.data[i].docid >= right.data[j].docid && j < right.num; j++) {
	    if (left.data[i].docid == right.data[j].docid) {

                if (TfIdf) {
                    left.data[i].score = left.data[i].score + right.data[j].score;
                } else {
                    /* Assign a large number, left or right */
                    left.data[i].score = left.data[i].score > right.data[j].score ?
                        left.data[i].score : right.data[j].score;
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

    for (; j < right.num; j++) {
	copy_hlist(val, v, right, j);
	v++;
    }

    free_hlist(left);
    free_hlist(right);
    val.num = v;
    return val;
}

void 
malloc_hlist(NmzResult * hlist, int n)
{
    if (n <= 0) return;
    hlist->data = malloc(n * sizeof(struct nmz_data));
    if (hlist->data == NULL) {
	 nmz_set_dyingmsg(nmz_msg("%s", strerror(errno)));
	 hlist->stat = ERR_FATAL;
	 return;
    }

    hlist->num  = n;
    hlist->stat = SUCCESS;
}

void 
realloc_hlist(NmzResult * hlist, int n)
{
    if (hlist->stat != SUCCESS || n <= 0) return;
    hlist->data = realloc(hlist->data, n * sizeof(struct nmz_data));
    if (hlist->data == NULL) {
	 nmz_set_dyingmsg(nmz_msg("%s", strerror(errno)));
	 hlist->stat = ERR_FATAL;
    }
}

void 
free_hlist(NmzResult hlist)
{
    if (hlist.stat != SUCCESS || hlist.num <= 0) return;
    free(hlist.data);
}

void 
copy_hlist(NmzResult to, int n_to, NmzResult from, int n_from)
{
    to.data[n_to] = from.data[n_from];
}

void 
set_idxid_hlist(NmzResult hlist, int id)
{
    int i;
    for (i = 0; i < hlist.num; i++) {
        hlist.data[i].idxid = id;
    }
}

NmzResult 
merge_hlist(NmzResult *hlists)
{
    int i, n;
    NmzResult value;

    if (get_idxnum() == 1) {
	return hlists[0];
    }

    for(i = n = 0; i < get_idxnum(); i++) {
        if (hlists[i].stat == SUCCESS && hlists[i].num > 0) {
            n += hlists[i].num;
        }
    }
    malloc_hlist(&value, n);
    if (value.stat == ERR_FATAL)
        return value;
    for(i = n = 0; i < get_idxnum(); i++) {
        if (hlists[i].stat != SUCCESS || hlists[i].num <= 0) 
            continue;
        memcpy_hlist(value, hlists[i], n);
        n += hlists[i].num;
        free_hlist(hlists[i]);
    }
    value.stat = SUCCESS;
    value.num = n;
    return value;
}

/*
 * Get date info from NMZ.t and do the missing number processing
 */
NmzResult 
do_date_processing(NmzResult hlist)
{
    FILE *date_index;
    int i;

    date_index = fopen(NMZ.t, "rb");
    if (date_index == NULL) {
	nmz_set_dyingmsg(nmz_msg("%s: %s", NMZ.t, strerror(errno)));
	hlist.stat = ERR_FATAL;
        return hlist; /* error */
    }

    for (i = 0; i < hlist.num ; i++) {
        if (-1 == fseek(date_index, hlist.data[i].docid * sizeof(hlist.data[i].date), 0)) {
	    nmz_set_dyingmsg(nmz_msg("%s: %s", NMZ.t, strerror(errno)));
	    hlist.stat = ERR_FATAL;
            return hlist; /* error */
        }
        nmz_fread(&hlist.data[i].date, sizeof(hlist.data[i].date), 1, date_index);

        if (hlist.data[i].date == -1) {  
            /* The missing number, this document has been deleted */
            int j;

            for (j = i + 1; j < hlist.num; j++) { /* shift */
                copy_hlist(hlist, j - 1, hlist, j);
            }
            hlist.num--;
            i--;
        }
    }

    fclose(date_index);
    return hlist;
}

/*
 * Get the hit list
 */
NmzResult 
get_hlist(int index)
{
    int n, *buf, i;
    NmzResult hlist;
    double idf = 0;
    hlist.num  = 0;
    hlist.stat = SUCCESS;

    if (-1 == fseek(Nmz.i, nmz_getidxptr(Nmz.ii, index), 0)) {
	hlist.stat = ERR_FATAL;
	return hlist; /* error */
    }

    nmz_get_unpackw(Nmz.i, &n);

    if (TfIdf) {
        idf = log((double)document_number / (n/2)) / log(2);
	nmz_debug_printf("idf: %f (N:%d, n:%d)\n", idf, document_number, n/2);
    }

    if (n >= IGNORE_HIT * 2) {  
        /* '* 2' means NMZ.i contains a file-ID and a score. */
        hlist.stat = ERR_TOO_MUCH_HIT;
    } else {
	int sum = 0;
	buf = malloc(n * sizeof(int)); /* with pelnty margin */
	if (buf == NULL) {
	    nmz_set_dyingmsg(nmz_msg("%s", strerror(errno)));
	    hlist.data = NULL;
	    hlist.stat = ERR_FATAL;
	    return hlist;
	}
	n = nmz_read_unpackw(Nmz.i, buf, n) / 2;
	malloc_hlist(&hlist, n);
	if (hlist.stat == ERR_FATAL)
	    return hlist;
	
	for (i = 0; i < n; i ++) {
	    hlist.data[i].docid = *(buf + i * 2) + sum;
	    sum = hlist.data[i].docid;
	    hlist.data[i].score = *(buf + i * 2 + 1);
            if (TfIdf) {
                hlist.data[i].score = (int)(hlist.data[i].score * idf) + 1;
            }
	}
        hlist.num = n;
	free(buf);
        hlist = do_date_processing(hlist);
    } 
    return hlist;
}


/*
 * Interface to invoke merge sort function
 */
int 
sort_hlist(NmzResult hlist, enum nmz_sort_method mode)
{
    set_rank(hlist); /* conserve current order for STABLE sorting */

    if (mode == SORT_BY_FIELD) {
	if (field_sort(hlist) != SUCCESS)
	    return FAILURE;
    } else if (mode == SORT_BY_DATE) {
	qsort(hlist.data, hlist.num, sizeof(hlist.data[0]), date_cmp);
    } else if (mode == SORT_BY_SCORE) {
	qsort(hlist.data, hlist.num, sizeof(hlist.data[0]), score_cmp);
    } 
    return 0;
}

/* 
 * reverse a given hlist
 * original of this routine was contributed by Furukawa-san [1997-11-13]
 */
int 
reverse_hlist(NmzResult hlist)
{
    int m, n;
    NmzResult tmp;

    malloc_hlist(&tmp, 1);
    if (tmp.stat == ERR_FATAL)
        return FAILURE;
    m = 0;
    n = hlist.num - 1;
    while (m < n) {
	copy_hlist(tmp, 0, hlist, m);
	copy_hlist(hlist, m, hlist, n);
	copy_hlist(hlist, n, tmp, 0);
	m++;
	n--;
    }
    free_hlist(tmp);
    return 0;
}

void 
set_docnum(int n)
{
    document_number = n;
}

void 
set_sortfield(const char *field)
{
    strcpy(field_for_sort, field);
}

char *
get_sortfield(void)
{
    return field_for_sort;
}

