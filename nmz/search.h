#ifndef _SEARCH_H
#define _SEARCH_H

extern int binsearch(char*, int);
extern NmzResult search_main(char*);
extern NmzResult do_search(char *, NmzResult);

extern void free_hitnums(struct nmz_hitnum *);

#endif /* _SEARCH_H */
