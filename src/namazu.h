#ifndef _NAMAZU_H
#define _NAMAZU_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdio.h>

#include "critical.h"
#include "magic.h"

/************************************************************
 *
 * Data Structures
 *
 ************************************************************/

typedef unsigned char uchar;

struct hlist_data {
    int score;  /* score */
    int docid;  /* document ID */
    int idxid;  /* index ID */
    int date; /* file's date */
    int rank; /* ranking data for stable sorting */
    char *field; /* for field-specified search*/

};
typedef struct hlist_data hlist_data;

/* data structure for search result */
struct hlist {
    int n;
    hlist_data *d;
};
typedef struct hlist HLIST;

struct replace_elem {
    struct replace_elem *next;
    uchar *src;
    uchar *dst;
    /* The following fields are NULL if this is a traditional
     * string substitution
     */
    struct re_pattern_buffer *src_exp;
    struct subst_elem {
	enum { literal, regex_regno } subst_type;
	union {
	    uchar *literal_string;
	    int regex_regno;
        } u;
    };
};

struct list {
    uchar *str;
    struct list *next;
};
typedef struct list LIST;

struct replace {
    LIST *src;
    LIST *dst;
};
typedef struct replace REPLACE;

struct alias {
    uchar *alias;
    uchar *real;
    struct alias *next;
};
typedef struct alias ALIAS;

/* NMZ.* files' names */
struct nmz_names {
#define MAXPATH 1024
    uchar i[MAXPATH];
    uchar ii[MAXPATH];
    uchar head[MAXPATH]; /* followed by a language code */
    uchar foot[MAXPATH]; /* followed by a language code */
    uchar body[MAXPATH]; /* followed by a language code */
    uchar lock[MAXPATH];
    uchar result[MAXPATH];
    uchar slog[MAXPATH];
    uchar w[MAXPATH];
    uchar wi[MAXPATH];
    uchar field[MAXPATH];  /* followed by a field name */
    uchar t[MAXPATH]; 
    uchar p[MAXPATH];
    uchar pi[MAXPATH];
};
typedef struct nmz_names NMZ_NAMES;

/* NMZ.* files' file pointers */
struct nmz_files {
    FILE *i;
    FILE *ii;
    FILE *p;
    FILE *pi;
    FILE *w;
    FILE *wi;
};
typedef struct nmz_files NMZ_FILES;

struct query {
    uchar str[BUFSIZE];              /* query string */
    uchar *tab[QUERY_TOKEN_MAX + 1];  /* table for the query string */
};
typedef struct query QUERY;

struct indices {
    int num;                 /* Number of indices */
    uchar *names[INDEX_MAX + 1]; /* Index names */
};
typedef struct indices INDICES;

#include "message.h"
#include "var.h"
#include "em.h"
 
#endif /* _NAMAZU_H */
