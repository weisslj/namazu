#ifndef _OUTPUT_H
#define _OUTPUT_H

#include "libnamazu.h"  /* for NmzResult struct */

extern void set_htmlmode ( int );
extern int is_htmlmode ( void );
extern void set_cgimode ( int );
extern int is_cgimode ( void );
extern void set_quietmode ( int );
extern int is_quietmode ( void );
extern int is_countmode ( void );
extern void set_countmode ( int );
extern void set_listmode ( int );
extern int is_listmode ( void );
extern void set_allresult ( int );
extern int is_allresult ( void );
extern void set_pageindex ( int );
extern int is_pageindex ( void );
extern void set_formprint ( int );
extern int is_formprint ( void );
extern void set_refprint ( int );
extern int is_refprint ( void );
extern void set_urireplace ( int );
extern int is_urireplace ( void );
extern void set_uridecode ( int );
extern int is_uridecode ( void );
extern void set_maxresult ( int );
extern int get_maxresult ( void );
extern void set_listwhence ( int );
extern int get_listwhence ( void );
extern void set_template ( char * );
extern char *get_template ( void );

extern void html_print(char*);
extern void put_current_range(int);
extern void put_page_index(int);
extern void print_msgfile(char*);

extern void print_hlist(NmzResult);

extern void print_result1(void);
extern void print_hitnum(int n);
extern void print_listing(NmzResult hlist);
extern void print_range(NmzResult hlist);
extern void print_hitnum_all_idx();

#endif /* _OUTPUT_H */
