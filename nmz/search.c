/*
 * 
 * search.c -
 * 
 * $Id: search.c,v 1.30 1999-12-09 08:33:48 satoru Exp $
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
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#ifdef __EMX__
#include <sys/types.h>
#endif
#include <sys/stat.h>

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "namazu.h"
#include "libnamazu.h"
#include "util.h"
#include "field.h"
#include "result.h"
#include "parser.h"
#include "hlist.h"
#include "re.h"
#include "wakati.h"
#include "output.h"
#include "search.h"
#include "i18n.h"
#include "codeconv.h"
#include "var.h"
#include "seed.h"

enum nmz_perm { ALLOW, DENY };
static int cur_idxnum = -1;


/*
 *
 * Private functions
 *
 */

static struct nmz_hitnum *push_hitnum ( struct nmz_hitnum *hn, int hitnum, enum nmz_stat stat, char *str );
void free_hitnums ( struct nmz_hitnum *hn );
static void show_status ( int l, int r );
static int get_file_size ( char *filename );
static void lrget ( char * key, int *l, int *r );
static NmzResult prefix_match ( char * orig_key, int v );
static enum nmz_search_mode detect_search_mode ( char *key );
static NmzResult do_word_search ( char *key, NmzResult val );
static NmzResult do_prefix_match_search ( char *key, NmzResult val );
static int hash ( char *str );
static NmzResult cmp_phrase_hash ( int hash_key, NmzResult val, FILE *phrase, FILE *phrase_index );
static int open_phrase_index_files ( FILE **phrase, FILE **phrase_index );
static NmzResult do_phrase_search ( char *key, NmzResult val );
static void do_regex_preprocessing ( char *expr );
static NmzResult do_regex_search ( char *orig_expr, NmzResult val );
static void get_expr ( char *expr, char *str );
static NmzResult do_field_search ( char *str, NmzResult val );
static void delete_beginning_backslash ( char *str );
static int check_lockfile ( void );
static enum nmz_perm parse_access ( char *line, char *rhost, char *raddr );
static enum nmz_perm check_access ( void );
static int open_index_files ( void );
static void close_index_files ( void );
static void do_logging ( char * query, int n );
static NmzResult search_sub ( NmzResult hlist, char *query, char *query_orig, int n );
static void make_fullpathname_index ( int n );

/* struct nmz_hitnum handling subroutines */
static struct nmz_hitnum *push_hitnum(struct nmz_hitnum *hn, 
				 int hitnum, 
				 enum nmz_stat stat,
				 char *str)
{
    struct nmz_hitnum *hnptr = hn, *prevhnptr = hn;
    while (hnptr != NULL) {
	prevhnptr = hnptr;
	hnptr = hnptr->next;
    }
    if ((hnptr = malloc(sizeof(struct nmz_hitnum))) == NULL) {
	set_dyingmsg("push_hitnum: malloc failed on hnptr");
	return NULL;
    }
    if (prevhnptr != NULL)
	prevhnptr->next = hnptr;
    hnptr->hitnum = hitnum;
    hnptr->stat  = stat;
    hnptr->phrase = NULL;
    hnptr->next  = NULL;
    if ((hnptr->word = malloc(strlen(str) +1)) == NULL) {
	set_dyingmsg("push_hitnum: malloc failed on str");
	return NULL;
    }
    strcpy(hnptr->word, str);

    if (hn == NULL)
	return hnptr;
    return hn;
}

void free_hitnums(struct nmz_hitnum *hn)
{
    struct nmz_hitnum *tmp;

    for (; hn != NULL; hn = tmp) {
	tmp = hn->next;
	free(hn->word);
	if (hn->phrase != NULL) { /* it has phrases */
	    free_hitnums(hn->phrase);
	}
	free(hn);
    }
}

/* show the status for debug use */
static void show_status(int l, int r)
{
    char buf[BUFSIZE];

    fseek(Nmz.w, getidxptr(Nmz.wi, l), 0);
    fgets(buf, BUFSIZE, Nmz.w);
    nmz_chomp(buf);

    nmz_debug_printf("l:%d: %s", l, buf);
    fseek(Nmz.w, getidxptr(Nmz.wi, r), 0);
    fgets(buf, BUFSIZE, Nmz.w);
    nmz_chomp(buf);

    nmz_debug_printf("r:%d: %s", r, buf);
}

/* get size of file */
static int get_file_size (char *filename) 
{
    struct stat st;

    stat(filename, &st);
    nmz_debug_printf("size of %s: %d\n", filename, (int)st.st_size);
    return ((int)(st.st_size));
}


/* get the left and right value of search range */
static void lrget(char * key, int *l, int *r)
{
    *l = 0;
    *r = get_file_size(NMZ.ii) / sizeof(int) - 1;

    if (is_debugmode()) {
	show_status(*l, *r);
    }
}

/* Prefix match search */
static NmzResult prefix_match(char * orig_key, int v)
{
    int i, j, n;
    NmzResult tmp, val;
    char buf[BUFSIZE], key[BUFSIZE];
    val.num = 0;

    strcpy(key, orig_key);
    key[strlen(key) - 1] = '\0';
    n = strlen(key);

    for (i = v; i >= 0; i--) {
	fseek(Nmz.w, getidxptr(Nmz.wi, i), 0);
	fgets(buf, BUFSIZE, Nmz.w);
	if (strncmp(key, buf, n) != 0) {
	    break;
	}
    }
    if (is_debugmode()) {
	v = i;
    }

    for (j = 0, i++;; i++, j++) {
	/* return if too much word would be hit
           because treat 'a*' completely is too consuming */
	if (j > IGNORE_MATCH) {
	    free_hlist(val);
	    val.stat = ERR_TOO_MUCH_MATCH;
	    break;
	}
	if (-1 == fseek(Nmz.w, getidxptr(Nmz.wi, i), 0)) {
	    break;
	}
	fgets(buf, BUFSIZE, Nmz.w);
        nmz_chomp(buf);
	if (strncmp(key, buf, n) == 0) {
	    tmp = get_hlist(i);
	    if (tmp.stat == ERR_FATAL)
	        return tmp;
	    if (tmp.num > IGNORE_HIT) {
		free_hlist(val);
		val.stat = ERR_TOO_MUCH_MATCH;
		break;
	    }
	    val = ormerge(val, tmp);
	    if (val.stat == ERR_FATAL)
	        return val;
	    if (val.num > IGNORE_HIT) {
		free_hlist(val);
		val.stat = ERR_TOO_MUCH_MATCH;
		break;
	    }
	    nmz_debug_printf("fw: %s, %d, %d\n", buf, tmp.num, val.num);
	} else
	    break;
    }
    nmz_debug_printf("range: %d - %d\n", v + 1, i - 1);
    return val;
}

/* detect search mode */
static enum nmz_search_mode detect_search_mode(char *key) {
    if (strlen(key) <= 1)
        return WORD_MODE;
    if (isfield(key)) { /* field search */
	nmz_debug_printf("do FIELD search\n");
        return FIELD_MODE;
    }
    if (*key == '/' && key[strlen(key) - 1] == '/') {
	nmz_debug_printf("do REGEX search\n");
	return REGEX_MODE;    /* regex match */
    } else if (*key == '*' 
               && key[strlen(key) - 1] == '*'
               && *(key + strlen(key) - 2) != '\\' ) 
    {
	nmz_debug_printf("do REGEX (INTERNAL_MATCH) search\n");
	return REGEX_MODE;    /* internal match search (treated as regex) */
    } else if (key[strlen(key) - 1] == '*'
        && *(key + strlen(key) - 2) != '\\')
    {
	nmz_debug_printf("do PREFIX_MATCH search\n");
	return PREFIX_MODE;    /* prefix match search */
    } else if (*key == '*') {
	nmz_debug_printf("do REGEX (SUFFIX_MATCH) search\n");
	return REGEX_MODE;    /* suffix match  (treated as regex)*/
    } else if ((*key == '"' && key[strlen(key) - 1] == '"') 
          || (*key == '{' && key[strlen(key) - 1] == '}')) 
    {
        /* remove the delimiter at begging and end of string */
        strcpy(key, key + 1); 
        key[strlen(key) - 1]= '\0';
    } 
    
    /* normal or phrase */

    /* if under Japanese mode, do wakatigaki */
    if (is_lang_ja()) {
        if (wakati(key))
	  return ERROR_MODE;
    }

    if (strchr(key, '\t')) {
	nmz_debug_printf("do PHRASE search\n");
	return PHRASE_MODE;
    } else {
	nmz_debug_printf("do WORD search\n");
        return WORD_MODE;
    }
}

static NmzResult do_word_search(char *key, NmzResult val)
{
    int v;

    if ((v = binsearch(key, 0)) != -1) {
        /* if found, get list */
        val = get_hlist(v);
	if (val.stat == ERR_FATAL)
	    return val;
    } else {
        val.num  = 0;  /* no hit */
	val.stat = SUCCESS; /* no hit but success */
	val.data = NULL;
    }
    return val;
}

static NmzResult do_prefix_match_search(char *key, NmzResult val)
{
    int v;

    if ((v = binsearch(key, 1)) != -1) { /* 2nd argument must be 1  */
        /* if found, do foward match */
        val = prefix_match(key, v);
	if (val.stat == ERR_FATAL)
	    return val;
    } else {
        val.num = 0;  /* no hit */
	val.data = NULL;
    }
    return val;
}


/* calculate a value of phase hash */
static int hash(char *str)
{
    int hash = 0, i, j;
    uchar *ustr = (uchar *)str;  /* for 8 bit chars handling */

    for (i = j = 0; *ustr; i++) {
        if (!issymbol(*ustr)) { /* except symbol */
            hash ^= nmz_seed[j & 0x3][*ustr];
            j++;
        }
        ustr++;
    }
    return (hash & 65535);
}

/* get the phrase hash and compare it with NmzResult */
static NmzResult cmp_phrase_hash(int hash_key, NmzResult val, 
                          FILE *phrase, FILE *phrase_index)
{
    int i, j, v, n, *list;
    long ptr;

    if (val.num == 0) {
        return val;
    }
    ptr = getidxptr(phrase_index, hash_key);
    if (ptr <= 0) {
	free_hlist(val);
        val.num = 0;
        return val;
    }
    fseek(phrase, ptr, 0);
    nmz_get_unpackw(phrase, &n);

    list = malloc(n * sizeof(int));
    if (list == NULL) {
	 set_dyingmsg("cmp_phrase_hash_malloc");
	 val.stat = ERR_FATAL;
	 return val;
    }

    {
	int docid, sum = 0;
	n = nmz_read_unpackw(phrase, list, n);
	for (i = j = v = 0; i < n; i++) {
	    docid = *(list + i) + sum;
	    sum = docid;
	    for (; j < val.num && docid >= val.data[j].docid; j++) {
		if (docid == val.data[j].docid) {
		    copy_hlist(val, v++, val, j);
		}
	    }
	}
    }
    if (v == 0) {
	free_hlist(val);
    }
    val.num = v;
    free(list);
    return val;
}

static int open_phrase_index_files(FILE **phrase, FILE **phrase_index)
{
    *phrase = fopen(NMZ.p, "rb");
    if (*phrase == NULL) {
        nmz_debug_printf("%s: cannot open file.\n", NMZ.p);
        return 1;
    }

    *phrase_index = fopen(NMZ.pi, "rb");
    if (*phrase_index == NULL) {
        nmz_debug_printf("%s: cannot open file.\n", NMZ.pi);
        return 1;
    }
    return 0;
}


/* phrase search */
static NmzResult do_phrase_search(char *key, NmzResult val)
{
    int i, h = 0, ignore = 0;
    char *p, *q, *word_b = 0, word_mix[BUFSIZE];
    FILE *phrase, *phrase_index;
    struct nmz_hitnum *pr_hitnum = NULL; /* phrase hitnum */

    p = key;
    if (strchr(p, '\t') == NULL) {  /* if only one word */
        val = do_word_search(p, val);
        return val;
    }

    if (open_phrase_index_files(&phrase, &phrase_index)) {
        val.stat = ERR_PHRASE_SEARCH_FAILED;  /* cannot open regex index */
        return val;
    }
        
    while (*p == '\t') {  /* beggining tabs are skipped */
        p++;
    }
    for (i = 0; ;i++) {
        q = (char *)strchr(p, '\t');
        if (q) {
            *q = '\0';
	}
        if (strlen(p) > 0) {
            NmzResult tmp;

            tmp = do_word_search(p, val);
	    if (tmp.stat == ERR_FATAL) 
	        return tmp;

	    pr_hitnum = push_hitnum(pr_hitnum, tmp.num, tmp.stat, p);
	    if (pr_hitnum == NULL) {
		tmp.stat = ERR_FATAL;
		return tmp;
	    }

            if (i == 0) {
                val = tmp;
            } else {
		if (tmp.stat == ERR_TOO_MUCH_HIT || 
		    val.stat == ERR_TOO_MUCH_HIT) {
		    ignore = 1;
		} else {
		    ignore = 0;
		}
                val = andmerge(val, tmp, &ignore);

		strcpy(word_mix, word_b);
		strcat(word_mix, p);
		h = hash(word_mix);
		val = cmp_phrase_hash(h, val, phrase, phrase_index);
		nmz_debug_printf("\nhash:: <%s, %s>: h:%d, val.num:%d\n",
			     word_b, p, h, val.num);
		if (val.num == 0) {
		    break;
		} else if (val.stat == ERR_FATAL) {
		    return val;
		}
	    }
	    word_b = p;
        }
        if (q == NULL) {
            break;
	}
        p = q + 1;
    }

    /* set phrase hit numbers using phrase member */
    {
	struct nmz_hitnum *x;

        /* set dummy */
	Idx.pr[cur_idxnum] = push_hitnum(Idx.pr[cur_idxnum], 
						 0, SUCCESS, "");
	if (Idx.pr[cur_idxnum] == NULL) {
	    val.stat = ERR_FATAL;
	    return val;
	}

	/* get the last element */
	x = Idx.pr[cur_idxnum];
	while (1) {
	    if (x->next == NULL) {
		break;
	    }
	    x = x->next;
	}
	x->phrase = pr_hitnum;
	x->hitnum = val.num;  /* total hit of phrases */
    }
    
    fclose(phrase);
    fclose(phrase_index);

    return val;
}

static void do_regex_preprocessing(char *expr)
{
    if (*expr == '*' && expr[strlen(expr) - 1] != '*') {
        /* if suffix match such as '*bar', enforce it into regex */
        strcpy(expr, expr + 1);
        strcat(expr, "$");
    } else if (*expr != '*' && expr[strlen(expr) - 1] == '*') {
        /* if prefix match such as 'bar*', enforce it into regex */
        expr[strlen(expr) - 1] = '.';
        strcat(expr, "*");
    } else if (*expr == '*' && expr[strlen(expr) - 1] == '*') {
        /* if internal match such as '*foo*', enforce it into regex */
        strcpy(expr, expr + 1);
        expr[strlen(expr) - 1] = '\0';
    } else if (*expr == '/' && expr[strlen(expr) - 1] == '/') {
        /* genuine regex */
        /* remove the both of '/' chars at begging and end of string */
        strcpy(expr, expr + 1); 
        expr[strlen(expr) - 1]= '\0';
        return;
    } else {
        char buf[BUFSIZE * 2], *bufp, *exprp;

        if ((*expr == '"' && expr[strlen(expr) - 1] == '"')
            || (*expr == '{' && expr[strlen(expr) - 1] == '}')) 
	{
            /* delimiters of field search */
            strcpy(expr, expr + 1); 
            expr[strlen(expr) - 1] = '\0';
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

static NmzResult do_regex_search(char *orig_expr, NmzResult val)
{
    FILE *fp;
    char expr[BUFSIZE * 2]; /* because of escaping meta characters */

    strcpy(expr, orig_expr);
    do_regex_preprocessing(expr);

    fp = fopen(NMZ.w, "rb");
    if (fp == NULL) {
        nmz_debug_printf("%s: cannot open file.\n", NMZ.w);
        val.stat = ERR_REGEX_SEARCH_FAILED;  /* cannot open regex index */
        return val;
    }
    val = regex_grep(expr, fp, "", 0);
    fclose(fp);
    return val;

}

static void get_expr(char *expr, char *str)
{
    str = (char *)strchr(str, (int)':') + 1;
    strcpy(expr, str);
}

static NmzResult do_field_search(char *str, NmzResult val)
{
    char expr[BUFSIZE * 2], /* because of escaping meta characters */
        field_name[BUFSIZE], file_name[BUFSIZE];
    FILE *fp;

    get_field_name(field_name, str);
    get_expr(expr, str);
    do_regex_preprocessing(expr);

    strcpy(file_name, NMZ.field); /* make pathname */
    strcat(file_name, field_name);

    fp = fopen(file_name, "rb");
    if (fp == NULL) {
        set_dyingmsg("%s: cannot open file.\n", file_name);
        val.stat = ERR_CANNOT_OPEN_INDEX;
        return val;
    }
    val = regex_grep(expr, fp, field_name, 1); /* last argument must be 1 */
    fclose(fp);
    return val;
}

static void delete_beginning_backslash(char *str)
{
    if (*str == '\\') {
        strcpy(str, str + 1);
    }
}

/* check the existence of lockfile */
static int check_lockfile(void)
{
    FILE *lock;

    if ((lock = fopen(NMZ.lock, "rb"))) {
	fclose(lock);
        print("(now be in system maintenance)");
        return 1;
    }
    return 0;
}


static enum nmz_perm parse_access(char *line, char *rhost, char *raddr)
{
    enum nmz_perm perm = ALLOW;

    /* Skip white spaces */
    line += strspn(line, " \t");

    if (*line == '\0' || *line == '#') {
	/* Ignore blank line or comment line */
        return perm;
    }
    if (strprefixcasecmp(line, "allow") == 0) {
	line += strlen("allow");
	line += strspn(line, " \t");
	if (strcasecmp(line, "all") == 0) {
	    perm = ALLOW;
	} else if (*raddr != '\0' && strprefixcmp(line, raddr) == 0) {
	    /* IP Address : forward match */
	    perm = ALLOW;
	} else if (*rhost != '\0' && strsuffixcmp(line, rhost) == 0) {
	    /* Hostname : backword match */
	    perm = ALLOW;
	}
    } else if (strprefixcasecmp(line, "deny") == 0) {
	line += strlen("deny");
	line += strspn(line, " \t");
	if (strcasecmp(line, "all") == 0) {
	    perm = DENY;
	} else if (*raddr != '\0' && strprefixcmp(line, raddr) == 0) {
	    /* IP Address : forward match */
	    perm = DENY;
	} else if (*rhost != '\0' && strsuffixcmp(line, rhost) == 0) {
	    /* Hostname : backword match */
	    perm = DENY;
	}
    }
    return perm;
}

static enum nmz_perm check_access(void)
{
    char buf[BUFSIZE];
    char *rhost, *raddr;
    FILE *fp;
    enum nmz_perm perm = ALLOW;
    
    rhost = safe_getenv("REMOTE_HOST");
    raddr = safe_getenv("REMOTE_ADDR");

    if (*rhost == '\0' && *raddr == '\0') { /* not from remote */
	return perm;
    }

    fp = fopen(NMZ.access, "rb");
    if (fp == NULL) {
	return perm;
    }
    while (fgets(buf, BUFSIZE, fp)) {
	nmz_chomp(buf);
	perm = parse_access(buf, rhost, raddr);
    }
    fclose(fp);
    return perm;
}

/* opening files at once */
static int open_index_files()
{
    if (check_lockfile())
        return 1;

    Nmz.i = fopen(NMZ.i, "rb");
    if (Nmz.i == NULL) {
	return 1;
    }
    Nmz.ii = fopen(NMZ.ii, "rb");
    if (Nmz.ii == NULL) {
	return 1;
    }
    Nmz.w = fopen(NMZ.w, "rb");
    if (Nmz.w == NULL) {
	return 1;
    }
    Nmz.wi = fopen(NMZ.wi, "rb");
    if (Nmz.wi == NULL) {
	return 1;
    }

    return 0;
}

/* closing files at once */
static void close_index_files(void)
{
    fclose(Nmz.i);
    fclose(Nmz.ii);
    fclose(Nmz.w);
    fclose(Nmz.wi);
}


/* do logging, separated with TAB characters 
   it does not consider a LOCK mechanism!
*/
static void do_logging(char * query, int n)
{
    FILE *slog;
    char *rhost;
    char *time_string;
    time_t t;

    t = time(&t);
    time_string = ctime(&t);

    slog = fopen(NMZ.slog, "a");
    if (slog == NULL) {
	nmz_warn_printf("%s: Permission denied\n", NMZ.slog);
	return;
    }
    rhost = safe_getenv("REMOTE_HOST");
    if (*rhost == '\0') {
	rhost = safe_getenv("REMOTE_ADDR");
    }
    if (*rhost == '\0')
	rhost = "LOCALHOST";
    fprintf(slog, "%s\t%d\t%s\t%s", query, n, rhost, time_string);

    fclose(slog);
}

static NmzResult search_sub(NmzResult hlist, char *query, char *query_orig, int n)
{
    cur_idxnum = n;

    if (check_access() != ALLOW) {
	/* if access denied */
	hlist.stat = ERR_NO_PERMISSION;
	return hlist;
    }

    if (open_index_files()) {
        /* if open failed */
        hlist.stat = ERR_CANNOT_OPEN_INDEX;
        return hlist;
    }

    /* if query contains only one keyword, TfIdf mode will be off */
    if (Query.tab[1] == NULL && strchr(Query.tab[0], '\t') == NULL)
        TfIdf = 0;
    if (TfIdf) {
	set_docnum(get_file_size(NMZ.t) / sizeof(int));
    }

    /* search */
    init_parser();
    hlist = expr();
    if (hlist.stat == ERR_FATAL) {
	set_dyingmsg("search error");
        return hlist;
    }

    if (hlist.stat == SUCCESS && hlist.num > 0) {  /* if hit */
        set_idxid_hlist(hlist, n);
    }
    Idx.total[cur_idxnum] = hlist.num;
    close_index_files();

    if (is_loggingmode()) {
        do_logging(query_orig, hlist.num);
    }
    return hlist;
}

static void make_fullpathname_index(int n)
{
    char *base;

    base = Idx.names[n];

    nmz_pathcat(base, NMZ.i);
    nmz_pathcat(base, NMZ.ii);
    nmz_pathcat(base, NMZ.w);
    nmz_pathcat(base, NMZ.wi);
    nmz_pathcat(base, NMZ.p);
    nmz_pathcat(base, NMZ.pi);
    nmz_pathcat(base, NMZ.lock);
    nmz_pathcat(base, NMZ.slog);
    nmz_pathcat(base, NMZ.field);
    nmz_pathcat(base, NMZ.t);
    nmz_pathcat(base, NMZ.access);
}


/*
 *
 * Public functions
 *
 */

/* main routine of binary search */
int binsearch(char *orig_key, int prefix_match_mode)
{
    int l, r, x, e = 0, i;
    char term[BUFSIZE], key[BUFSIZE];

    strcpy(key, orig_key);
    lrget(key, &l, &r);

    if (prefix_match_mode) {  /* truncate a '*' character at the end */
        key[strlen(key) - 1] = '\0';
    }

    while (r >= l) {
	x = (l + r) / 2;

	/* over BUFSIZE (maybe 1024) size keyword is nuisance */
	fseek(Nmz.w, getidxptr(Nmz.wi, x), 0);
	fgets(term, BUFSIZE, Nmz.w);
	nmz_chomp(term);

	nmz_debug_printf("searching: %s", term);
	for (e = 0, i = 0; *(term + i) != '\0' && *(key + i) != '\0' ; i++) {
	    /*
	     * compare in unsigned. 
	     * because they could be 8 bit characters (0x80 or upper).
	     */
	    if ((uchar)*(term + i) > (uchar)*(key + i)) {
		e = -1;
		break;
	    }
	    if ((uchar)*(term + i) < (uchar)*(key + i)) {
		e = 1;
		break;
	    }
	}

	if (*(term + i) == '\0' && *(key + i)) {
	    e = 1;
	} else if (! prefix_match_mode && *(term + i) != '\0' 
                   && (*(key + i) == '\0')) {
	    e = -1;
	}

	/* if hit, return */
	if (e == 0) {
	    return x;
	}

	if (e < 0) {
	    r = x - 1;
	} else {
	    l = x + 1;
	}
    }
    return -1;
}

/* flow of search */
NmzResult nmz_search(char *query)
{
    NmzResult hlist, tmp[INDEX_MAX];
    char query_orig[BUFSIZE];
    int i, ret;

    if (strlen(query) > QUERY_MAX) {
	hlist.stat = ERR_TOO_LONG_QUERY;
	return hlist;
    }

    strcpy(query_orig, query); /* save */

    ret = split_query(query);
    if (ret != SUCCESS) {
	hlist.stat = ret;
	return hlist;
    }

    for (i = 0; i < Idx.num; i++) {
        make_fullpathname_index(i);
        tmp[i] = search_sub(tmp[i], query, query_orig, i);

	if (tmp[i].stat == ERR_FATAL) {
	    hlist.data = NULL;
	    hlist.stat = ERR_FATAL;
	    return hlist; /* need freeing memory? */
	}
    }


    hlist = merge_hlist(tmp);

    if (hlist.stat == SUCCESS && hlist.num > 0) { /* HIT!! */
	/* sort by date at first*/
        if (sort_hlist(hlist, SORT_BY_DATE) != SUCCESS) {
	    hlist.stat = ERR_FATAL;
	    return hlist;
	}
	if (get_sortmethod() != SORT_BY_DATE) {
	    if (sort_hlist(hlist, get_sortmethod()) != SUCCESS) {
	        hlist.stat = ERR_FATAL;
		return hlist;
	    }
	}
        if (get_sortorder() == ASCENDING) {  /* default is descending */
	    if (reverse_hlist(hlist)) {
	        hlist.stat = ERR_FATAL;
		return hlist; 
	    }
        }
    }

    return hlist;
}



NmzResult do_search(char *orig_key, NmzResult val)
{
    enum nmz_search_mode mode;
    char key[BUFSIZE];

    strcpy(key, orig_key);
    strlower(key);
    mode = detect_search_mode(key);
    if (mode == ERROR_MODE) {
	val.stat = ERR_FATAL;
	return val;
    }
    delete_beginning_backslash(key);

    if (mode == PREFIX_MODE) {
        val = do_prefix_match_search(key, val);
    } else  if (mode == REGEX_MODE) {
        val = do_regex_search(key, val);
    } else if (mode == PHRASE_MODE) {
        val = do_phrase_search(key, val);
    } else if (mode == FIELD_MODE) {
        val = do_field_search(key, val);
    } else {
        val = do_word_search(key, val);
    }

    if (mode != PHRASE_MODE) {
	struct nmz_hitnum *prtmp;

	prtmp = push_hitnum(Idx.pr[cur_idxnum], 
			    val.num, val.stat, orig_key);
	if (prtmp == NULL) {
	    val.stat = ERR_FATAL;
	    return val;
	}
	Idx.pr[cur_idxnum] = prtmp;
    }

    return val;
}
