#ifndef _MAGIC_H
#define _MAGIC_H

/************************************************************
 *
 * Magic Numbers
 *
 ************************************************************/

enum {
    ESC    = 0x1b,      /* Code of ESC */
    STDIN  = 0,		/* stdin's  fd */
    STDOUT = 1,		/* stdout's fd */
    STDERR = 2		/* stderr's fd */
};

enum {
    SORT_BY_SCORE,  /* at displaying results time */
    SORT_BY_DATE,   /* at displaying results time */
    SORT_BY_FIELD   /* at displaying results time */
};

enum {
    ASCENDING,     /* Direction of sorting */
    DESCENDING     /* Direction of sorting */
};

enum {
    ERR_TOO_MUCH_MATCH,
    ERR_TOO_MUCH_HIT,
    ERR_REGEX_SEARCH_FAILED,
    ERR_PHRASE_SEARCH_FAILED
};

/* error code */
enum {
    DIE_SET_REDIRECT_TO_FILE = -3,
    DIE_ERROR = -2,
    DIE_NOERROR = -1,

    DIE_HLIST = -10
};

#endif /* _MAGIC_H */
