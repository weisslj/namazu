#ifndef _CGI_H
#define _CGI_H

struct cgivar {
    uchar *name;
    uchar *value;
    struct cgivar *next;
};
typedef struct cgivar CGIVAR;

/*
 * cgiarg: data structure for passing process_cgi_var_*() functions 
 * as a second arg as a pointer.
 */
struct cgiarg {
    uchar *query;
    uchar *subquery;
};
typedef struct cgiarg CGIARG;


struct cgivar_func {
    uchar *name;
    void (*func)(uchar*, CGIARG*);
};
typedef struct cgivar_func CGIVAR_FUNC;

void init_cgi(uchar*, uchar*);

#endif /* _CGI_H */
