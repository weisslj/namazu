#ifndef _QUERY_H
#define _QUERY_H

extern enum nmz_stat make_query(const char *querystring);
extern int get_querytokennum(void);
char *get_querytoken(int id);

#endif /* _QUERY_H */
