/*
 * 
 * libnamazu.h - Namazu library api
 *
 * $Id: libnamazu.h,v 1.12 1999-11-23 12:58:31 satoru Exp $
 * 
 */

#ifndef _LIBNAMAZU_H
#define _LIBNAMAZU_H

#include <stdio.h>     /* for FILE struct */

/*
 *
 * Critical values
 *
 */

enum {
    BUFSIZE = 1024,        /* Size of general buffers */

    QUERY_TOKEN_MAX =  16, /* Max number of tokens in a query */
    QUERY_MAX       = 256, /* Max length of an IR query */

    IGNORE_HIT    = 10000, /* Ignore if pages matched more than this */
    IGNORE_MATCH  = 1000,  /* Ignore if words matched more than this */

    INDEX_MAX = 64        /* Max number of databases */
};


/* for error handling */
enum {
    SUCCESS,
    ERR_TOO_LONG_QUERY,
    ERR_INVALID_QUERY,
    ERR_TOO_MANY_TOKENS
};

/*
 *
 * Data Structures
 *
 */

typedef unsigned char uchar;

typedef struct hlist_data {
    int score;   /* score */
    int docid;   /* document ID */
    int idxid;   /* index ID */
    int date;    /* document's date */
    int rank;    /* ranking data for stable sorting */
    int status;  /* mainly used for storing error status code */
    char *field; /* for field-specified search*/
} HLIST_DATA;

/* data structure for search result */
typedef struct hlist {
    int n;
    int status;  /* mainly used for storing error status code */
    HLIST_DATA *d;
} HLIST;

typedef struct alias {
    char *alias;
    char *real;
    struct alias *next;
} ALIAS;

typedef struct re_pattern_buffer REGEX;

typedef struct REPLACE {
    struct replace *next;
    char  *pat;  /* pattern */
    char  *rep;  /* replacement */
    REGEX  *pat_re; /* compiled regex of the pattern */
} REPLACE;

/* NMZ.* files' names */
typedef struct nmz_names {
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
} NMZ_NAMES;

/* NMZ.* files' file pointers */
typedef struct nmz_files {
    FILE *i;
    FILE *ii;
    FILE *p;
    FILE *pi;
    FILE *w;
    FILE *wi;
} NMZ_FILES;

typedef struct query {
    char str[BUFSIZE];              /* query string */
    char *tab[QUERY_TOKEN_MAX + 1];  /* table for the query string */
} QUERY;

/* results of phrase search */
typedef struct phraseres {
    int hitnum;
    char *word;
    struct phraseres *next;
} PHRASERES;

typedef struct indices {
    int num;                 /* Number of indices */
    char *names[INDEX_MAX + 1]; /* Index names */

    int total[INDEX_MAX + 1]; /* results of total hits */
    int mode[INDEX_MAX + 1]; /* search mode */
    PHRASERES *pr[INDEX_MAX + 1]; /* results of each word hits */
    int phrasehit[INDEX_MAX + 1]; /* results of each phrase hits */
} INDICES;

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

