#ifndef _RE_H
#define _RE_H

#define ALLOC_N(type,n) (type*)xmalloc(sizeof(type)*(n))
#define ALLOC(type) (type*)xmalloc(sizeof(type))
#define MEMZERO(p,type,n) memset((p), 0, sizeof(type)*(n))

int replace_uri(char*);
HLIST regex_grep(char*, FILE*, char*, int);

#endif /* _RE_H */
