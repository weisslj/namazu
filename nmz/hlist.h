#ifndef _NmzResult_H
#define _NmzResult_H

#include "libnamazu.h"  /* for NmzResult and enum nmz_* */

extern NmzResult andmerge(NmzResult, NmzResult, int*);
extern NmzResult notmerge(NmzResult, NmzResult, int*);
extern NmzResult ormerge(NmzResult, NmzResult);
extern void malloc_hlist(NmzResult*, int);
extern void realloc_hlist(NmzResult* , int);
extern void free_hlist(NmzResult);
extern NmzResult do_date_processing(NmzResult);
extern NmzResult get_hlist(int);
extern void nmz_mergesort(int, int, NmzResult, NmzResult, int);
extern int sort_hlist(NmzResult, enum nmz_sort_method);
extern int reverse_hlist(NmzResult);
extern NmzResult merge_hlist(NmzResult*);
extern void set_idxid_hlist(NmzResult, int);
extern void copy_hlist(NmzResult, int, NmzResult, int);
extern void set_docnum(int);
extern void set_sort_field(char *);
extern char *get_sort_field(void);

#endif /* _NmzResult_H */
