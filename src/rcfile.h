#ifndef _CONF_H
#define _CONF_H

#include "util.h"  /* For struct nmz_strlist. */

#define DIRECTIVE_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"

struct conf_directive {
    char *name;
    int  argnum;
    int  plus;   /* argnum or more arguments are required. */
    enum nmz_stat (*func)(const char *directive,
			  const struct nmz_strlist *args);
};

extern char *set_namazurc ( const char *arg );
extern enum nmz_stat load_rcfile ( const char *av0 );
extern void show_rcfile ( void );

#endif /* _CONF_H */
