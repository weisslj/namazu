/*
 * 
 * libnamazu.h - Namazu library api
 *
 * $Id: libnamazu.h,v 1.11 1999-11-23 09:46:18 satoru Exp $
 * 
 */

#ifndef _LIBNAMAZU_H
#define _LIBNAMAZU_H

#include <stdio.h>  /* for FILE struct */
#include "critical.h"  /* for INDEX_MAX */

/*
 *
 * Data Structures
 *
 */

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
    char *str;
    struct list *next;
};
typedef struct list LIST;

struct alias {
    char *alias;
    char *real;
    struct alias *next;
};
typedef struct alias ALIAS;

typedef struct re_pattern_buffer REGEX;
struct replace {
    struct replace *next;
    char  *pat;  /* pattern */
    char  *rep;  /* replacement */
    REGEX  *pat_re; /* compiled regex of the pattern */
};
typedef struct replace REPLACE;

/* NMZ.* files' names */
struct nmz_names {
#define MAXPATH 1024
    char i[MAXPATH];
    char ii[MAXPATH];
    char head[MAXPATH]; /* followed by a language code */
    char foot[MAXPATH]; /* followed by a language code */
    char body[MAXPATH]; /* followed by a language code */
    char lock[MAXPATH];
    char result[MAXPATH];
    char slog[MAXPATH];
    char w[MAXPATH];
    char wi[MAXPATH];
    char field[MAXPATH];  /* followed by a field name */
    char t[MAXPATH]; 
    char p[MAXPATH];
    char pi[MAXPATH];
    char tips[MAXPATH];
    char access[MAXPATH];
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
    char str[BUFSIZE];              /* query string */
    char *tab[QUERY_TOKEN_MAX + 1];  /* table for the query string */
};
typedef struct query QUERY;

/* results of phrase search */
struct phraseres {
    int hitnum;
    char *word;
    struct phraseres *next;
};

typedef struct phraseres PHRASERES;

struct indices {
    int num;                 /* Number of indices */
    char *names[INDEX_MAX + 1]; /* Index names */

    int total[INDEX_MAX + 1]; /* results of total hits */
    int mode[INDEX_MAX + 1]; /* search mode */
    PHRASERES *pr[INDEX_MAX + 1]; /* results of each word hits */
    int phrasehit[INDEX_MAX + 1]; /* results of each phrase hits */
};
typedef struct indices INDICES;

extern void free_idxnames(void);
extern void free_aliases(void);
extern void free_replaces(void);
extern void make_fullpathname_msg(void);
extern void codeconv_query(char *query);
extern void getenv_namazurc(void);
extern void uniq_idxnames(void);
extern int expand_idxname_aliases(void);
extern int complete_idxnames(void);
extern char *set_namazurc(char *arg);
extern void set_sortmethod ( int );
extern int get_sortmethod ( void );
extern void set_sortorder ( int );
extern int get_sortorder ( void );
extern void set_debugmode ( int );
extern int is_debugmode ( void );
extern void set_loggingmode ( int );
extern int is_loggingmode ( void );
extern void set_dyingmsg ( char *, ... );
extern char *get_dyingmsg ( void );

#endif /* _LIBNAMAZU_H */
