#ifndef _MAGIC_H
#define _MAGIC_H

/*
 *
 * Magic Numbers
 *
 */

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

enum {  /* all should be negative value. */
    ERR_TOO_MUCH_MATCH       = -1,
    ERR_TOO_MUCH_HIT         = -2,
    ERR_REGEX_SEARCH_FAILED  = -3,
    ERR_PHRASE_SEARCH_FAILED = -4,
    ERR_CANNOT_OPEN_INDEX    = -5,
    ERR_NO_PERMISSION        = -6
};

/* error code */
enum {
    DIE_SET_REDIRECT_TO_FILE = -3,
    DIE_ERROR = -2,
    DIE_NOERROR = -1,

    DIE_HLIST = -255
};

#endif /* _MAGIC_H */
