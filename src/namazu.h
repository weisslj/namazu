#ifndef _NAMAZU_H
#define _NAMAZU_H

enum {
    PAGE_MAX            = 20,  /* Max number of result pages */
    RESULT_MAX          = 100, /* Max number of result displays at once */
    CGI_QUERY_MAX       = 512, /* Max length of a CGI query */
    CGI_INDEX_NAME_MAX  = 64,  /* Max length of an index name in a CGI query */
    CGI_RESULT_NAME_MAX = 64,  /* Max length of a result name in a CGI query */

    SUICIDE_TIME = 60      /* namazu.cgi will suicide when its
			      processing time exceeds this */
};

extern int main(int argc, char **argv);
extern void die(const char *fmt, ...);

#endif /* _NAMAZU_H */
