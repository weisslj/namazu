/*
 * 
 * search.c -
 * 
 * $Id: search.c,v 1.10 1999-08-25 07:09:24 satoru Exp $
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
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#ifdef __EMX__
#include <sys/types.h>
#endif
#include <sys/stat.h>

#include "namazu.h"
#include "util.h"
#include "field.h"
#include "result.h"
#include "parser.h"
#include "hlist.h"
#include "re.h"
#include "wakati.h"
#include "output.h"
#include "search.h"

/************************************************************
 *
 * Private functions
 *
 ************************************************************/

void show_status(int, int);
int get_file_size (uchar*);
void lrget(uchar* , int*, int*);
HLIST forward_match(uchar* , int);
int detect_search_mode(uchar*);
void print_hit_count (uchar*, HLIST);
HLIST do_word_search(uchar*, HLIST);
HLIST do_forward_match_search(uchar*, HLIST);
int hash(uchar*);
HLIST cmp_phrase_hash(int, HLIST, FILE *, FILE *);
int open_phrase_index_files(FILE**, FILE**);
HLIST do_phrase_search(uchar*, HLIST);
void do_regex_preprocessing(uchar*);
HLIST do_regex_search(uchar*, HLIST);
void get_expr(uchar*, uchar*);
HLIST do_field_search(uchar*, HLIST);
void delete_beginning_backslash(uchar*);
int check_lockfile(void);
void check_byte_order(void);
int open_index_files();
void close_index_files(void);
void print_hitnum(int);
void print_range(HLIST);
char *get_env_safely(char*);
void do_logging(uchar* , int);
uchar *get_dir_name(uchar*);
HLIST search_sub(HLIST, uchar*, uchar*, int);
void make_fullpathname_index(int);


/* show the status for debug use */
void show_status(int l, int r)
{
    uchar buf[BUFSIZE];

    fseek(Index, getidxptr(IndexIndex, l), 0);
    fgets(buf, BUFSIZE, Index);
    fprintf(stderr, "l:%d: %s", l, buf);
    fseek(Index, getidxptr(IndexIndex, r), 0);
    fgets(buf, BUFSIZE, Index);
    fprintf(stderr, "r:%d: %s", r, buf);
}

/* get size of file */
int get_file_size (uchar *filename) {
    struct stat st;

    stat(filename, &st);
    if (Debug) {
        fprintf(stderr, "size of %s: %d\n", filename, (int)st.st_size);
    }
    return ((int)(st.st_size));
}


/* get the left and right value of search range */
void lrget(uchar * key, int *l, int *r)
{
    *l = 0;
    *r = get_file_size(INDEXINDEX) / sizeof(int) - 1;

    if (Debug)
	show_status(*l, *r);
}

/* Forward match search */
HLIST forward_match(uchar * orig_key, int v)
{
    int i, j, n;
    HLIST tmp, val;
    uchar buf[BUFSIZE], key[BUFSIZE];
    val.n = 0;

    strcpy(key, orig_key);
    *(lastc(key))= '\0';
    n = strlen(key);

    for (i = v; i >= 0; i--) {
	fseek(Index, getidxptr(IndexIndex, i), 0);
	fgets(buf, BUFSIZE, Index);
	if (strncmp(key, buf, n))
	    break;
    }
    if (Debug)
	v = i;

    for (j = 0, i++;; i++, j++) {
	/* return if too much word would be hit
           because treat 'a*' completely is too consuming */
	if (j > IGNORE_MATCH) {
	    free_hlist(val);
	    val.n = TOO_MUCH_MATCH;
	    break;
	}
	if (-1 == fseek(Index, getidxptr(IndexIndex, i), 0))
	    break;
	fgets(buf, BUFSIZE, Index);
        chomp(buf);
	if (!strncmp(key, buf, n)) {
	    tmp = get_hlist(i);
	    if (tmp.n > IGNORE_HIT) {
		free_hlist(val);
		val.n = TOO_MUCH_MATCH;
		break;
	    }
	    val = ormerge(val, tmp);
	    if (val.n > IGNORE_HIT) {
		free_hlist(val);
		val.n = TOO_MUCH_MATCH;
		break;
	    }
	    if (Debug)
		fprintf(stderr, "fw: %s, %d, %d\n", buf, tmp.n, val.n);
	} else
	    break;
    }
    if (Debug)
	fprintf(stderr, "range: %d - %d\n", v + 1, i - 1);
    return val;
}

#define NM 0
#define FW 1
#define RE 2
#define PH 3
#define FI 4

/* detect search mode */
int detect_search_mode(uchar *key) {
    if (strlen(key) <= 1)
        return NM;
    if (isfield(key)) { /* field search */
        if (Debug)
            fprintf(stderr, "do FIELD search\n");
        return FI;
    }
    if (*key == '/' && *(lastc(key)) == '/') {
        if (Debug)
            fprintf(stderr, "do REGEX search\n");
	return RE;    /* regex match */
    } else if (*key == '*' 
               && *(lastc(key)) == '*'
               && *(key + strlen(key) - 2) != '\\' ) 
    {
        if (Debug)
            fprintf(stderr, "do REGEX (INTERNAL_MATCH) search\n");
	return RE;    /* internal match search (treated as regex) */
    } else if (*(lastc(key)) == '*'
        && *(key + strlen(key) - 2) != '\\')
    {
        if (Debug)
            fprintf(stderr, "do FORWARD_MATCH search\n");
	return FW;    /* forward match search */
    } else if (*key == '*') {
        if (Debug)
            fprintf(stderr, "do REGEX (BACKWARD_MATCH) search\n");
	return RE;    /* backward match  (treated as regex)*/
    } else if ((*key == '"' && *(lastc(key)) == '"') 
          || (*key == '{' && *(lastc(key)) == '}')) 
    {
        /* remove the delimiter at begging and end of string */
        strcpy(key, key + 1); 
        *(lastc(key))= '\0';
    } 
    
    /* normal or phrase */

    /* if under Japanese mode, do wakatigaki */
    if (is_lang_ja(Lang)) {
        wakati(key);
    }

    if (strchr(key, '\t')) {
        if (Debug)
            fprintf(stderr, "do PHRASE search\n");
	return PH;
    } else {
        if (Debug)
            fprintf(stderr, "do WORD search\n");
        return NM;
    }
}

void print_hit_count (uchar *key, HLIST val)
{
    if (!HitCountOnly && !MoreShortFormat && !NoReference && !Quiet) {
        printf(" [ ");
        fputx(key, stdout);
        if (val.n > 0) {
            printf(": %d", val.n);
        } else { 
            uchar *msg = (uchar *)"";
            if (val.n == 0) {
                msg = (uchar *)": 0 ";
            } else if (val.n == TOO_MUCH_MATCH) {
                msg = MSG_TOO_MUCH_MATCH;
            } else if (val.n == TOO_MUCH_HIT) {
                msg = MSG_TOO_MUCH_HIT;
            } else if (val.n == REGEX_SEARCH_FAILED) {
                msg = MSG_CANNOT_OPEN_REGEX_INDEX;
            } 
            fputx(msg, stdout);
        }
        printf(" ] ");
    }
}

HLIST do_word_search(uchar *key, HLIST val)
{
    int v;

    if ((v = binsearch(key, 0)) != -1) {
        /* if found, get list */
        val = get_hlist(v);
    } else {
        val.n = 0;  /* no hit */
    }
    return val;
}

HLIST do_forward_match_search(uchar *key, HLIST val)
{
    int v;

    if ((v = binsearch(key, 1)) != -1) { /* 2nd argument must be 1  */
        /* if found, do foward match */
        val = forward_match(key, v);
    } else {
        val.n = 0;  /* no hit */
    }
    return val;
}


/* calculate a value of phase hash */
int hash(uchar *str)
{
    extern int Seed[4][256];
    int hash = 0, i, j;

    for (i = j = 0; *str; i++) {
        if (!issymbol(*str)) { /* except symbol */
            hash ^= Seed[j % 4][(int)*str];
            j++;
        }
        str++;
    }
    return (hash & 65535);
}

/* get the phrase hash and compare it with HLIST */
HLIST cmp_phrase_hash(int hash_key, HLIST val, 
                          FILE *phrase, FILE *phrase_index)
{
    int i, j, v, n, *list;
    long ptr;

    if (val.n == 0) {
        return val;
    }
    ptr = getidxptr(phrase_index, hash_key);
    if (ptr <= 0) {
        val.n = 0;
        return val;
    }
    fseek(phrase, ptr, 0);
    get_unpackw(phrase, &n);

    list = (int *)malloc(n * sizeof(int));
    if (list == NULL) {
	 die("cmp_phrase_hash_malloc");
    }

    {
	int fid, sum = 0;
	n = read_unpackw(phrase, list, n);
	for (i = j = v = 0; i < n; i++) {
	    fid = *(list + i) + sum;
	    sum = fid;
	    for (; j < val.n && fid >= val.fid[j]; j++) {
		if (fid == val.fid[j]) {
		    copy_hlist(val, v++, val, j);
		}
	    }
	}
    }
    val.n = v;
    free(list);
    return val;
}

int open_phrase_index_files(FILE **phrase, FILE **phrase_index)
{
    *phrase = fopen(PHRASE, "rb");
    if (*phrase == NULL) {
        return 1;
    }

    *phrase_index = fopen(PHRASEINDEX, "rb");
    if (*phrase_index == NULL) {
        return 1;
    }
    return 0;
}


/* phrase search */
HLIST do_phrase_search(uchar *key, HLIST val)
{
    int i, h = 0, ignore = 0, no_phrase_index = 0;
    uchar *p, *q, *word_b = 0, word_mix[BUFSIZE];
    FILE *phrase, *phrase_index;

    p = key;
    if (strchr(p, '\t') == NULL) {  /* if only one word */
        val = do_word_search(p, val);
        return val;
    }

    if (open_phrase_index_files(&phrase, &phrase_index)) {
/*        fputx(MSG_CANNOT_OPEN_PHRASE_INDEX, stdout); */
	no_phrase_index = 1;
    }
        
    if (!HitCountOnly && !MoreShortFormat && !NoReference && !Quiet) {
        printf(" { ");
    }
    while (*p == '\t') {  /* beggining tabs are skipped */
        p++;
    }
    for (i = 0; ;i++) {
        q = (uchar *)strchr(p, '\t');
        if (q) 
            *q = '\0';
        if (strlen(p) > 0) {
            HLIST tmp;

            tmp = do_word_search(p, val);
            print_hit_count(p, tmp);

            if (i == 0) {
                val = tmp;
            } else {
                val = andmerge(val, tmp, &ignore);
            }
	    if (!no_phrase_index) {
		if (i != 0) {
		    strcpy(word_mix, word_b);
		    strcat(word_mix, p);
		    h = hash(word_mix);
		    val = cmp_phrase_hash(h, val, phrase, phrase_index);
		    if (Debug) {
			fprintf(stderr, "\nhash:: <%s, %s>: h:%d, val.n:%d\n",
				word_b, p, h, val.n);
		    }
		}
		word_b = p;
	    }
        }
        if (q == NULL)
            break;
        p = q + 1;
    }
    if (!HitCountOnly && !MoreShortFormat && !NoReference && !Quiet) {
        printf(" :: %d } ", val.n);
    }

    if (!no_phrase_index) {
	fclose(phrase);
	fclose(phrase_index);
    }

    return val;
}

#define iseuc(c)  ((c) >= 0xa1 && (c) <= 0xfe)
void do_regex_preprocessing(uchar *expr)
{
    if (*expr == '*' && *(lastc(expr)) != '*') {
        /* if backward match such as '*bar', enforce it into regex */
        strcpy(expr, expr + 1);
        strcat(expr, "$");
    } else if (*expr != '*' && *(lastc(expr)) == '*') {
        /* if forward match such as 'bar*', enforce it into regex */
        *(lastc(expr)) = '.';
        strcat(expr, "*");
    } else if (*expr == '*' && *(lastc(expr)) == '*') {
        /* if internal match such as '*foo*', enforce it into regex */
        strcpy(expr, expr + 1);
        *(lastc(expr)) = '\0';
    } else if (*expr == '/' && *(lastc(expr)) == '/') {
        /* genuine regex */
        /* remove the both of '/' chars at begging and end of string */
        strcpy(expr, expr + 1); 
        *(lastc(expr))= '\0';
        return;
    } else {
        uchar buf[BUFSIZE * 2], *bufp, *exprp;

        if ((*expr == '"' && *(lastc(expr)) == '"')
            || (*expr == '{' && *(lastc(expr)) == '}')) {
            /* delimiters of field search */
            strcpy(expr, expr + 1); 
            *(lastc(expr))= '\0';
        }
        bufp = buf;
        exprp = expr;
        /* escape meta characters */
        while (*exprp) {
            if (!isalnum(*exprp) && !iseuc(*exprp)) {
                *bufp = '\\';
                bufp++;
            }
            *bufp = *exprp;
            bufp++;
            exprp++;
        }
        *bufp = '\0';
        strcpy(expr, buf);
    }
}

HLIST do_regex_search(uchar *orig_expr, HLIST val)
{
    FILE *fp;
    uchar expr[BUFSIZE * 2]; /* because of escaping meta characters */

    strcpy(expr, orig_expr);
    do_regex_preprocessing(expr);

    fp = fopen(WORDLIST, "rb");
    if (fp == NULL) {
        fprintf(stderr, "%s: cannot open file.\n", WORDLIST);
        val.n = REGEX_SEARCH_FAILED;  /* cannot open regex index */
        return val;
    }
    val = regex_grep(expr, fp, "", 0);
    fclose(fp);
    return val;

}

void get_expr(uchar *expr, uchar *str)
{
    str = (uchar *)strchr(str, (int)':') + 1;
    strcpy(expr, str);
}

HLIST do_field_search(uchar *str, HLIST val)
{
    uchar expr[BUFSIZE * 2], /* because of escaping meta characters */
        field_name[BUFSIZE], file_name[BUFSIZE];
    FILE *fp;

    get_field_name(field_name, str);
    get_expr(expr, str);
    do_regex_preprocessing(expr);

    strcpy(file_name, FIELDINFO); /* make pathname */
    strcat(file_name, field_name);

    fp = fopen(file_name, "rb");
    if (fp == NULL) {
        fprintf(stderr, "%s: cannot open file.\n", file_name);
        val.n = -4;  /* cannot open field index */
        return val;
    }
    val = regex_grep(expr, fp, field_name, 1); /* last argument must be 1 */
    fclose(fp);
    return val;
}

void delete_beginning_backslash(uchar *str)
{
    if (*str == '\\') {
        strcpy(str, str + 1);
    }
}

/* check the existence of lockfile */
int check_lockfile(void)
{
    FILE *lock;

    if ((lock = fopen(LOCKFILE, "rb"))) {
	fclose(lock);
        printf("(now be in system maintenance)");
        return 1;
    }
    return 0;
}


/* checking byte order */
void check_byte_order(void)
{
    int  n = 1;
    char *c;
    
    OppositeEndian = 0;
    c = (char *)&n;
    if (*c) { /* little-endian */
	OppositeEndian = 1;
    } 
    if (Debug && OppositeEndian) {
        fprintf(stderr, "OppositeEndian mode\n");
    }
}

/* opening files at once */
int open_index_files()
{
    if (check_lockfile())
        return 1;
    check_byte_order();
    Index = fopen(INDEX, "rb");
    if (Index == NULL) {
	return 1;
    }
    IndexIndex = fopen(INDEXINDEX, "rb");
    if (IndexIndex == NULL) {
	return 1;
    }
    return 0;
}

/* closing files at once */
void close_index_files(void)
{
    fclose(Index);
    fclose(IndexIndex);
}


/* flow of displaying search results */
void print_result1(void)
{
    fputx(MSG_RESULT_HEADER, stdout);

    if (HtmlOutput)
	fputs("<p>\n", stdout);
    else
	fputc('\n', stdout);
    fputx(MSG_REFERENCE_HEADER, stdout);
    if (DbNumber > 1 && HtmlOutput)
	fputs("</p>\n", stdout);
}

void print_result2(void)
{
    if (DbNumber == 1 && HtmlOutput)
	printf("\n</p>\n");
    else
	fputc('\n', stdout);
}

void print_hitnum(int n)
{
    fputx(MSG_HIT_1, stdout);
    if (HtmlOutput)
        printf("<!-- HIT -->%d<!-- HIT -->", n);
    else
        printf("%d", n);
    fputx(MSG_HIT_2, stdout);
}

void print_listing(HLIST hlist)
{
    if (HtmlOutput)
        printf("<dl>\n");
    
    print_hlist(hlist);
    
    if (HtmlOutput)
        printf("</dl>\n");

}

void print_range(HLIST hlist)
{
    if (HtmlOutput)
        printf("<p>\n");
    put_current_range(hlist.n);
    if (!HidePageIndex)
        put_page_index(hlist.n);
    if (HtmlOutput)
        printf("</p>\n");
    else
        printf("\n");
}

char *get_env_safely(char *s)
{
    char *cp;
    return (cp = getenv(s)) ? cp : "";
}


/* do logging, separated with TAB characters 
   it does not consider a LOCK mechanism!
*/
void do_logging(uchar * query, int n)
{
    FILE *slog;
    char *rhost;
    char *time_string;
    time_t t;

    t = time(&t);
    time_string = ctime(&t);

    slog = fopen(SLOG, "a");
    if (slog == NULL) {
        if (Debug)
            fprintf(stderr, "NMZ.slog: Permission denied\n");
	return;
    }
    rhost = get_env_safely("REMOTE_HOST");
    if (!*rhost)
	rhost = get_env_safely("REMOTE_ADDR");
    if (!*rhost)
	rhost = "LOCALHOST";
    fprintf(slog, "%s\t%d\t%s\t%s", query, n, rhost, time_string);

    fclose(slog);
}


uchar *get_dir_name(uchar *path)
{
    uchar *p;

    p = (uchar *)strrchr(path, '/');
    if (p) {
        return (p + 1);
    } else {
        return path;
    }
}

HLIST search_sub(HLIST hlist, uchar *query, uchar *query_orig, int n)
{
    if (!HitCountOnly && !MoreShortFormat && !NoReference && !Quiet) {
        if (DbNumber > 1) {
            if (HtmlOutput)
                printf("<li><strong>%s</strong>: ", get_dir_name(DbNames[n]));
            else
                printf("(%s)", DbNames[n]);
        }
    }

    if (open_index_files()) {
        /* if open failed */
        hlist.n = 0;
        fputx(MSG_CANNOT_OPEN_INDEX, stdout);
        return hlist;
    }

    /* if query contains only one keyword, TfIdf mode will be off */
    if (KeyItem[1] == NULL && strchr(KeyItem[0], '\t') == NULL)
        TfIdf = 0;
    if (TfIdf) {
        AllDocumentN = get_file_size(DATEINDEX) / sizeof(int);
    }

    /* search */
    init_parser();
    hlist = expr();

    if (hlist.n) /* if hit */
        set_did_hlist(hlist, n);
    if (!HitCountOnly && !MoreShortFormat && !NoReference && !Quiet) {
        if (DbNumber > 1 && KeyItem[1]) {
            printf(" [ TOTAL: %d ]", hlist.n);
        }
        printf("\n");
    }
    close_index_files();

    if (Logging) {
        do_logging(query_orig, hlist.n);
    }
    return hlist;
}

void make_fullpathname_index(int n)
{
    uchar *base;

    base = DbNames[n];

    pathcat(base, INDEX);
    pathcat(base, INDEXINDEX);
    pathcat(base, WORDLIST);
    pathcat(base, PHRASE);
    pathcat(base, PHRASEINDEX);
    pathcat(base, LOCKFILE);
    pathcat(base, SLOG);
    pathcat(base, FIELDINFO);
    pathcat(base, DATEINDEX);
}


/************************************************************
 *
 * Public functions
 *
 ************************************************************/

/* main routine of binary search */
int binsearch(uchar *orig_key, int forward_match_mode)
{
    int l, r, x, e = 0, i;
    uchar buf[BUFSIZE], key[BUFSIZE];

    strcpy(key, orig_key);
    lrget(key, &l, &r);

    if (forward_match_mode) {  /* truncate a '*' character at the end */
        *(lastc(key)) = '\0';
    }

    while (r >= l) {
	x = (l + r) / 2;

	fseek(Index, getidxptr(IndexIndex, x), 0);

	/* over BUFSIZE (maybe 1024) size keyword is nuisance */
	fgets(buf, BUFSIZE, Index);
	if (Debug)
	    fprintf(stderr, "searching: %s", buf);
	for (e = 0, i = 0; *(buf + i) != '\n' && *(key + i) != '\0' ; i++) {
	    if (*(buf + i) > *(key + i)) {
		e = -1;
		break;
	    }
	    if (*(buf + i) < *(key + i)) {
		e = 1;
		break;
	    }
	}

	if (*(buf + i) == '\n' && *(key + i)) {
	    e = 1;
	} else if (! forward_match_mode && *(buf + i) != '\n' 
                   && (!*(key + i))) {
	    e = -1;
	}

	/* if hit, return */
	if (!e)
	    return x;

	if (e < 0)
	    r = x - 1;
	else
	    l = x + 1;
    }
    return -1;
}

/* flow of search */
void search_main(uchar *query)
{
    HLIST hlist, tmp[DB_MAX];
    uchar query_orig[BUFSIZE];
    int i;

    strcpy(query_orig, query); /* save */
    split_query(query);

    if (!HitCountOnly && !MoreShortFormat && !NoReference && !Quiet) {
        print_result1();

        if (DbNumber > 1) {
            printf("\n");
            if (HtmlOutput)
                printf("<ul>\n");
        }
    }
    for (i = 0; i < DbNumber; i++) {
        make_fullpathname_index(i);
        tmp[i] = search_sub(tmp[i], query, query_orig, i);
    }
    if (!HitCountOnly && !MoreShortFormat && !NoReference && !Quiet) {
        if (DbNumber > 1 && HtmlOutput) {
            printf("</ul>\n");
        }
        print_result2();
    }

    hlist = merge_hlist(tmp);
    if (hlist.n > 0) {       /* HIT!! */
        reverse_hlist(hlist);
        sort_hlist(hlist, SORT_BY_DATE);
        if (! LaterOrder) {  /* early order */
            reverse_hlist(hlist);
        }
        if (ScoreSort) {
            sort_hlist(hlist, SORT_BY_SCORE);
        }
        if (!HitCountOnly && !MoreShortFormat && !Quiet) {
            print_hitnum(hlist.n);  /* <!-- HIT -->%d<!-- HIT --> */
        }
	if (HitCountOnly) {
	    printf("%d\n", hlist.n);
	} else {
	    print_listing(hlist); /* summary listing */
	}
        if (!HitCountOnly && !MoreShortFormat && !Quiet) {
            print_range(hlist);
        }
    } else {
        if (HitCountOnly) {
            printf("0\n");
        } else if (!MoreShortFormat) {
            fputx(MSG_NO_HIT, stdout);
        }
    }
    free_hlist(hlist);
}



HLIST do_search(uchar *orig_key, HLIST val)
{
    int mode;
    uchar key[BUFSIZE];

    strcpy(key, orig_key);
    strlower(key);
    mode = detect_search_mode(key);
    delete_beginning_backslash(key);

    if (mode == FW) {
        val = do_forward_match_search(key, val);
    } else  if (mode == RE) {
        val = do_regex_search(key, val);
    } else if (mode == PH) {
        val = do_phrase_search(key, val);
    } else if (mode == FI) {
        val = do_field_search(key, val);
    } else {
        val = do_word_search(key, val);
    }

    if (mode != PH) { /* phrase mode print status by itself */
        print_hit_count(orig_key, val);
    }
    return val;
}


