#ifndef _OUTPUT_H
#define _OUTPUT_H

#include "libnamazu.h"  /* for HLIST struct */

extern void html_print(char*);
extern void put_current_range(int);
extern void put_page_index(int);
extern void print_msgfile(char*);

extern void print_hlist(HLIST);

extern void print_result1(void);
extern void print_hitnum(int n);
extern void print_listing(HLIST hlist);
extern void print_range(HLIST hlist);
extern void print_hit_count();

#endif /* _OUTPUT_H */
