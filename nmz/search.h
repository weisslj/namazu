#ifndef _SEARCH_H
#define _SEARCH_H

extern int binsearch(char*, int);
extern HLIST search_main(char*);
extern HLIST do_search(char *, HLIST);

extern void free_phraseres(PHRASERES *);

#endif /* _SEARCH_H */
