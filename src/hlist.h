#ifndef _HLIST_H
#define _HLIST_H

HLIST andmerge(HLIST, HLIST, int*);
HLIST notmerge(HLIST, HLIST, int*);
HLIST ormerge(HLIST, HLIST);
void malloc_hlist(HLIST*, int);
void realloc_hlist(HLIST* , int);
void free_hlist(HLIST);
HLIST do_date_processing(HLIST);
HLIST get_hlist(int);
void nmz_mergesort(int, int, HLIST, HLIST, int);
void sort_hlist(HLIST, int);
void reverse_hlist(HLIST);
HLIST merge_hlist(HLIST*);
void set_idxid_hlist(HLIST, int);
void copy_hlist(HLIST, int, HLIST, int);
void set_docnum(int);
void set_sort_field(uchar *);
uchar *get_sort_field(void);

#endif /* _HLIST_H */
