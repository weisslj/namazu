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
    STDERR = 2,		/* stderr's fd */

    SORT_BY_SCORE = 1,  /* at displaying results time */
    SORT_BY_DATE  = 2,  /* at displaying results time */
    SORT_BY_FIELD = 3,  /* at displaying results time */

    ASCENDING  = 0,     /* Direction of sorting */
    DESCENDING = 1,     /* Direction of sorting */

    ERR_TOO_MUCH_MATCH       = -1,
    ERR_TOO_MUCH_HIT         = -2,
    ERR_REGEX_SEARCH_FAILED  = -3,
    ERR_PHRASE_SEARCH_FAILED = -4
};

#endif /* _MAGIC_H */
