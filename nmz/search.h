#ifndef _SEARCH_H
#define _SEARCH_H

#include "libnamazu.h" /* for NmzResult type */

extern void nmz_free_hitnums ( struct nmz_hitnumlist *hn );
extern int nmz_binsearch ( const char *key, int prefix_match_mode );
extern NmzResult nmz_search ( const char *query );
extern NmzResult nmz_do_searching ( const char *key, NmzResult val );
extern void nmz_set_tfidfmode(int mode);
extern int  nmz_is_tfidfmode(void);

#endif /* _SEARCH_H */


