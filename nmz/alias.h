#ifndef _ALIAS_H
#define _ALIAS_H

#include "libnamazu.h" /* for enum nmz_stat */

struct nmz_alias {
    char *alias;
    char *real;
    struct nmz_alias *next;
};

extern enum nmz_stat add_alias ( const char *alias, const char *real );
extern void free_aliases ( void );
extern struct nmz_alias *get_aliases ( void );
extern void show_aliases ( void );

#endif /* _ALIAS_H */
