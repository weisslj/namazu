#ifndef _OUTPUT_H
#define _OUTPUT_H

void html_print(uchar*);
void put_current_range(int);
void put_page_index(int);
void print_msgfile(uchar*);

void print_hlist(HLIST);

void print_result1(void);
void print_hitnum(int n);
void print_listing(HLIST hlist);
void print_range(HLIST hlist);
void print_hit_count();

#endif /* _OUTPUT_H */
