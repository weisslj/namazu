#ifndef _REPLACE_H
#define _REPLACE_H

#include "libnamazu.h" /* for enum nmz_stat */

struct nmz_replace {
    char  *pat;  /* pattern */
    char  *rep;  /* replacement */
    struct re_pattern_buffer *pat_re; /* compiled regex of the pattern */
    struct nmz_replace *next;
};

extern void nmz_free_replaces ( void );
extern int replace_uri ( char *uri );
extern enum nmz_stat add_replace ( const char *pat, const char *rep );
extern void show_replaces ( void );

#endif /* _REPLACE_H */
