#ifndef _CGI_H
#define _CGI_H

struct cgivar {
    char *name;
    char *value;
    struct cgivar *next;
};
typedef struct cgivar CGIVAR;

/*
 * cgiarg: data structure for passing process_cgi_var_*() functions 
 * as a second arg as a pointer.
 */
struct cgiarg {
    char *query;
    char *subquery;
};
typedef struct cgiarg CGIARG;


struct cgivar_func {
    char *name;
    void (*func)(char*, CGIARG*);
};
typedef struct cgivar_func CGIVAR_FUNC;

extern void init_cgi(char*, char*);

#endif /* _CGI_H */
