#ifndef _NAMAZU_H
#define _NAMAZU_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdio.h>

#if ENABLE_NLS
# include <libintl.h>
# define _(Text) gettext (Text)
#else
# undef bindtextdomain
# define bindtextdomain(Domain, Directory) /* empty */
# undef textdomain
# define textdomain(Domain) /* empty */
# define _(Text) Text
#endif
#define N_(Text) Text

#include "critical.h"
#include "magic.h"
#include "regex.h"

/************************************************************
 *
 * Data Structures
 *
 ************************************************************/

typedef unsigned char uchar;

struct HLIST_DATA {
    int score;   /* score */
    int docid;   /* document ID */
    int idxid;   /* index ID */
    int date;    /* document's date */
    int rank;    /* ranking data for stable sorting */
    char *field; /* for field-specified search*/
};
typedef struct HLIST_DATA HLIST_DATA;

/* data structure for search result */
struct hlist {
    int n;
    HLIST_DATA *d;
};
typedef struct hlist HLIST;

struct list {
    uchar *str;
    struct list *next;
};
typedef struct list LIST;

struct alias {
    uchar *alias;
    uchar *real;
    struct alias *next;
};
typedef struct alias ALIAS;

typedef struct re_pattern_buffer REGEX;
struct replace {
    struct replace *next;
    uchar  *pat;  /* pattern */
    uchar  *rep;  /* replacement */
    REGEX  *pat_re; /* compiled regex of the pattern */
};
typedef struct replace REPLACE;

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
    uchar tips[MAXPATH];
    uchar access[MAXPATH];
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

/* results of phrase search */
struct phraseres {
  int hitnum;
  uchar *word;
  struct phraseres *next;
};

typedef struct phraseres PHRASERES;

struct indices {
    int num;                 /* Number of indices */
    uchar *names[INDEX_MAX + 1]; /* Index names */

    int total[INDEX_MAX + 1]; /* results of total hits */
    int mode[INDEX_MAX + 1]; /* search mode */
    PHRASERES *pr[INDEX_MAX + 1]; /* results of each word hits */
    int phrasehit[INDEX_MAX + 1]; /* results of each phrase hits */
};
typedef struct indices INDICES;

#include "message.h"
#include "var.h"
 
#endif /* _NAMAZU_H */
