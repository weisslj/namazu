#ifndef _HLIST_H
#define _HLIST_H

#include "libnamazu.h"  /* for NmzResult and enum nmz_* */

/*
 *   This file was automatically generated by version 1.7 of cextract.
 *   Manual editing not recommended.
 *
 *   Created: Fri Dec 10 08:34:29 1999
 */
#ifndef __CEXTRACT__
#if __STDC__

extern NmzResult andmerge ( NmzResult left, NmzResult right, int *ignore );
extern NmzResult notmerge ( NmzResult left, NmzResult right, int *ignore );
extern NmzResult ormerge ( NmzResult left, NmzResult right );
extern void malloc_hlist ( NmzResult * hlist, int n );
extern void realloc_hlist ( NmzResult * hlist, int n );
extern void free_hlist ( NmzResult hlist );
extern void copy_hlist ( NmzResult to, int n_to, NmzResult from, int n_from );
extern void set_idxid_hlist ( NmzResult hlist, int id );
extern NmzResult merge_hlist ( NmzResult *hlists );
extern NmzResult do_date_processing ( NmzResult hlist );
extern NmzResult get_hlist ( int index );
extern int sort_hlist ( NmzResult hlist, enum nmz_sort_method mode );
extern int reverse_hlist ( NmzResult hlist );
extern void set_docnum ( int n );
extern void set_sortfield ( char *field );
extern char *get_sortfield ( void );

#else /* __STDC__ */

extern NmzResult andmerge (/* NmzResult left, NmzResult right, int *ignore */);
extern NmzResult notmerge (/* NmzResult left, NmzResult right, int *ignore */);
extern NmzResult ormerge (/* NmzResult left, NmzResult right */);
extern void malloc_hlist (/* NmzResult * hlist, int n */);
extern void realloc_hlist (/* NmzResult * hlist, int n */);
extern void free_hlist (/* NmzResult hlist */);
extern void copy_hlist (/* NmzResult to, int n_to, NmzResult from, int n_from */);
extern void set_idxid_hlist (/* NmzResult hlist, int id */);
extern NmzResult merge_hlist (/* NmzResult *hlists */);
extern NmzResult do_date_processing (/* NmzResult hlist */);
extern NmzResult get_hlist (/* int index */);
extern int sort_hlist (/* NmzResult hlist, enum nmz_sort_method mode */);
extern int reverse_hlist (/* NmzResult hlist */);
extern void set_docnum (/* int n */);
extern void set_sortfield (/* char *field */);
extern char *get_sortfield (/* void */);

#endif /* __STDC__ */
#endif /* __CEXTRACT__ */
#endif /* _HLIST_H */
