/*
 * 
 * libnamazu.h - Namazu library api
 *
 * $Id: libnamazu.h,v 1.33 2000-01-08 09:27:19 satoru Exp $
 * 
 */

#ifndef _LIBNAMAZU_H
#define _LIBNAMAZU_H

#include <stdio.h>   /* for FILE struct */


/*
 * Set the message of error occurred in libnmz.
 */
#define nmz_set_dyingmsg(msg) \
    { \
        if (is_debugmode()) { \
            nmz_set_dyingmsg_sub("%s:%d: %s", __FILE__, __LINE__, msg);\
        } else { \
            nmz_set_dyingmsg_sub("%s", msg);\
	} \
    }

/*
 *
 * Critical values.
 *
 */

enum {
    /* Size of general buffers. This MUST be larger than QUERY_MAX */
    BUFSIZE = 1024,        

    QUERY_TOKEN_MAX =  32, /* Max number of tokens in a query */
    QUERY_MAX       = 256, /* Max length of an IR query */

    IGNORE_HIT    = 10000, /* Ignore if pages matched more than this */
    IGNORE_MATCH  = 1000,  /* Ignore if words matched more than this */

    INDEX_MAX = 64        /* Max number of databases */
};


/*
 *
 * Magic Numbers.
 *
 */

enum {
    ESC    = 0x1b,      /* Code of ESC */
    STDIN  = 0,		/* stdin's  fd */
    STDOUT = 1,		/* stdout's fd */
    STDERR = 2		/* stderr's fd */
};

/*
 *
 * Data Structures.
 *
 */

typedef unsigned char uchar;

/* 
 * Status code for error handling.
 */
enum nmz_stat {
    FAILURE = -1,
    SUCCESS,
    ERR_FATAL,
    ERR_TOO_LONG_QUERY,
    ERR_INVALID_QUERY,
    ERR_TOO_MANY_TOKENS,
    ERR_TOO_MUCH_MATCH,
    ERR_TOO_MUCH_HIT,
    ERR_REGEX_SEARCH_FAILED,
    ERR_PHRASE_SEARCH_FAILED,
    ERR_FIELD_SEARCH_FAILED,
    ERR_CANNOT_OPEN_INDEX,
    ERR_NO_PERMISSION,
    ERR_CANNOT_OPEN_RESULT_FORMAT_FILE
};

/* 
 * Modes of searching.
 */
enum nmz_search_mode {
    WORD_MODE,
    PREFIX_MODE,
    REGEX_MODE,
    PHRASE_MODE,
    FIELD_MODE,
    ERROR_MODE
};


/* 
 * Methods of sorting.
 */
enum nmz_sort_method {
    SORT_BY_SCORE,
    SORT_BY_DATE,
    SORT_BY_FIELD
};

/* 
 * Orders of sorting.
 */
enum nmz_sort_order {
    ASCENDING,
    DESCENDING
};


/* 
 * Data structure for each hit document.
 */
struct nmz_data {
    int score;
    int docid;   /* document ID */
    int idxid;   /* index ID */
    int date;    /* document's date */
    int rank;    /* ranking data for stable sorting */
    char *field; /* field's contents for field-specified search */
};

/* 
 * Data structure for search result 
 */
typedef struct nmz_result {
    int num;           /* number of elements in its data */
    enum nmz_stat stat; /* status code mainly used for error handling */
    struct nmz_data *data;  /* dynamic array for storing nmz_data's. */
} NmzResult;


/* 
 * NMZ.* files' names 
 */
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

/* 
 * NMZ.* files' file pointers 
 */
struct nmz_files {
    FILE *i;
    FILE *ii;
    FILE *p;
    FILE *pi;
    FILE *w;
    FILE *wi;
};

struct nmz_indices {
    int num;                    /* Number of indices */
    char *names[INDEX_MAX + 1]; /* Index names */
    struct nmz_hitnumlist 
    *hitnumlists[INDEX_MAX + 1];       /* hitnum list of each index */
    int totalhitnums[INDEX_MAX + 1];   /* total hit number of each index */
};

struct nmz_query {
    int  tokennum;                   /* Number of tokens in the query */
    char str[BUFSIZE];               /* Query string */
    char *tab[QUERY_TOKEN_MAX + 1];  /* Token pointers pointed to str */
};


/* 
 * List containig hit numbers of each index word by word.
 */
struct nmz_hitnumlist {
    char *word;
    int hitnum;
    enum nmz_stat stat;        /* status code mainly used for error handling */
    struct nmz_hitnumlist *phrase; /* for a result of a phrase search */
    struct nmz_hitnumlist *next;
};

extern void free_aliases ( void );
extern void free_replaces ( void );
extern void codeconv_query ( char *query );
extern void set_sortmethod ( enum nmz_sort_method method );
extern enum nmz_sort_method get_sortmethod ( void );
extern void set_sortorder ( enum nmz_sort_order order );
extern enum nmz_sort_order get_sortorder ( void );
extern void set_debugmode ( int mode );
extern int is_debugmode ( void );
extern void set_loggingmode ( int mode );
extern int is_loggingmode ( void );
extern char *nmz_get_dyingmsg ( void );
extern char *nmz_set_dyingmsg_sub(const char *fmt, ...);
extern char *nmz_msg(const char *fmt, ...);

#endif /* _LIBNAMAZU_H */
