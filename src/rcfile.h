#ifndef _RCFILE_H
#define _RCFILE_H

#define DIRECTIVE_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"

extern enum nmz_stat load_rcfiles ( void );
extern enum nmz_stat load_rcfile ( const char *fname );
extern void show_config ( void );
extern void set_namazurc ( const char *arg );

#endif /* _RCFILE_H */
