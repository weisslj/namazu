#ifndef _CGI_H
#define _CGI_H

enum {
    CGI_QUERY_MAX       = 1024,/* Max length of a CGI query */
    CGI_INDEX_NAME_MAX  =  128,/* Max length of an index name in a CGI query */
    CGI_RESULT_NAME_MAX =  128 /* Max length of an index name in a CGI query */
};


void init_cgi(uchar*, uchar*);

#endif /* _CGI_H */
