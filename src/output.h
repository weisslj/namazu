#ifndef _OUTPUT_H
#define _OUTPUT_H

#include "libnamazu.h"  /* for NmzResult struct */

/*
 *   This file was automatically generated by version 1.7 of cextract.
 *   Manual editing not recommended.
 *
 *   Created: Thu Dec  9 10:52:24 1999
 */
#ifndef __CEXTRACT__
#if __STDC__

extern enum nmz_stat print_result ( NmzResult hlist, char *query, char *subquery );
extern void print_default_page ( void );
extern void set_htmlmode ( int mode );
extern int is_htmlmode ( void );
extern void set_cgimode ( int mode );
extern int is_cgimode ( void );
extern void set_quietmode ( int mode );
extern int is_quietmode ( void );
extern int is_countmode ( void );
extern void set_countmode ( int mode );
extern void set_listmode ( int mode );
extern int is_listmode ( void );
extern void set_allresult ( int mode );
extern void set_pageindex ( int mode );
extern void set_formprint ( int mode );
extern int is_formprint ( void );
extern void set_refprint ( int mode );
extern int is_refprint ( void );
extern void set_maxresult ( int num );
extern int get_maxresult ( void );
extern void set_listwhence ( int num );
extern int get_listwhence ( void );
extern void set_template ( char *tmpl );
extern char *get_template ( void );
extern void html_print ( char *str );
extern void print_msgfile ( char *fname );
extern void print_hitnum_all_idx ( void );
extern void print_hitnum ( int n );
extern void print_listing ( NmzResult hlist );
extern void print_range ( NmzResult hlist );

#else /* __STDC__ */

extern enum nmz_stat print_result (/* NmzResult hlist, char *query, char *subquery */);
extern void print_default_page (/* void */);
extern void set_htmlmode (/* int mode */);
extern int is_htmlmode (/* void */);
extern void set_cgimode (/* int mode */);
extern int is_cgimode (/* void */);
extern void set_quietmode (/* int mode */);
extern int is_quietmode (/* void */);
extern int is_countmode (/* void */);
extern void set_countmode (/* int mode */);
extern void set_listmode (/* int mode */);
extern int is_listmode (/* void */);
extern void set_allresult (/* int mode */);
extern void set_pageindex (/* int mode */);
extern void set_formprint (/* int mode */);
extern int is_formprint (/* void */);
extern void set_refprint (/* int mode */);
extern int is_refprint (/* void */);
extern void set_maxresult (/* int num */);
extern int get_maxresult (/* void */);
extern void set_listwhence (/* int num */);
extern int get_listwhence (/* void */);
extern void set_template (/* char *tmpl */);
extern char *get_template (/* void */);
extern void html_print (/* char *str */);
extern void print_msgfile (/* char *fname */);
extern void print_hitnum_all_idx (/* void */);
extern void print_hitnum (/* int n */);
extern void print_listing (/* NmzResult hlist */);
extern void print_range (/* NmzResult hlist */);

#endif /* __STDC__ */
#endif /* __CEXTRACT__ */
#endif /* _OUTPUT_H */
