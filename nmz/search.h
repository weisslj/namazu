#ifndef _SEARCH_H
#define _SEARCH_H

/* modes of searching */
enum {
    WORD_MODE,
    PREFIX_MODE,
    REGEX_MODE,
    PHRASE_MODE,
    FIELD_MODE,

    ERROR_MODE = -1
};

int binsearch(char*, int);
HLIST search_main(char*);
HLIST do_search(char *, HLIST);

void free_phraseres(PHRASERES *);

#endif /* _SEARCH_H */
