#ifndef _SEARCH_H
#define _SEARCH_H

#include "libnamazu.h" /* for NmzResult type */

extern void free_hitnums ( struct nmz_hitnum *hn );
extern int binsearch ( char *orig_key, int prefix_match_mode );
extern NmzResult nmz_search ( char *query );
extern NmzResult do_search ( char *orig_key, NmzResult val );

#endif /* _SEARCH_H */
