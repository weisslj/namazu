#ifndef _SEARCH_H
#define _SEARCH_H

#include "libnamazu.h" /* for NmzResult type */

extern void free_hitnums ( struct nmz_hitnumlist *hn );
extern int binsearch ( const char *key, int prefix_match_mode );
extern NmzResult nmz_search ( const char *query );
extern NmzResult do_search ( const char *key, NmzResult val );

#endif /* _SEARCH_H */
