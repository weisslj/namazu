#ifndef _HLIST_H
#define _HLIST_H

extern HLIST andmerge(HLIST, HLIST, int*);
extern HLIST notmerge(HLIST, HLIST, int*);
extern HLIST ormerge(HLIST, HLIST);
extern void malloc_hlist(HLIST*, int);
extern void realloc_hlist(HLIST* , int);
extern void free_hlist(HLIST);
extern HLIST do_date_processing(HLIST);
extern HLIST get_hlist(int);
extern void nmz_mergesort(int, int, HLIST, HLIST, int);
extern int sort_hlist(HLIST, int);
extern int reverse_hlist(HLIST);
extern HLIST merge_hlist(HLIST*);
extern void set_idxid_hlist(HLIST, int);
extern void copy_hlist(HLIST, int, HLIST, int);
extern void set_docnum(int);
extern void set_sort_field(char *);
extern char *get_sort_field(void);

#endif /* _HLIST_H */
