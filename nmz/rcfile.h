#ifndef _CONF_H
#define _CONF_H

#define DIRECTIVE_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"

extern char *set_namazurc ( char *arg );
extern enum nmz_stat load_rcfile ( char *av0 );
extern void show_rcfile ( void );

#endif /* _CONF_H */
