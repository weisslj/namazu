#ifndef _OUTPUT_H
#define _OUTPUT_H

#include "libnamazu.h"  /* for NmzResult struct */

extern enum nmz_stat print_result ( NmzResult hlist, const char *query, const char *subquery );
extern void print_default_page ( void );
extern void set_htmlmode ( int mode );
extern int is_htmlmode ( void );
extern void set_cgimode ( int mode );
extern void set_quietmode ( int mode );
extern void set_countmode ( int mode );
extern void set_listmode ( int mode );
extern void set_allresult ( int mode );
extern void set_pageindex ( int mode );
extern void set_formprint ( int mode );
extern int is_formprint ( void );
extern void set_refprint ( int mode );
extern void set_maxresult ( int num );
extern int get_maxresult ( void );
extern void set_listwhence ( int num );
extern int get_listwhence ( void );
extern void set_template ( const char *tmpl );
extern char *get_template ( void );
extern void html_print ( const char *str );
extern void print ( const char *str );

#endif /* _OUTPUT_H */
