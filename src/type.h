#ifndef _TYPE_H
#define _TYPE_H

#include <stdio.h>
typedef unsigned char uchar;


/* data structure for search result */
struct hlist {
    int n;
    int *scr;  /* score */
    int *fid;  /* file ID */
    int *did;  /* database (index) ID */
    int *date; /* file's date */
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
};
typedef struct nmz_files NMZ_FILES;

#endif /* _TYPE_H */

