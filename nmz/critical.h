#ifndef _VALUE_H
#define _VALUE_H

/************************************************************
 *
 * Critical values
 *
 ************************************************************/

enum {
    BUFSIZE = 1024,        /* Size of general buffers */

    QUERY_TOKEN_MAX =  16, /* Max number of tokens in a query */
    QUERY_MAX       = 256, /* Max length of an IR query */
    PAGE_MAX = 20,	   /* Max number of result pages */

    IGNORE_HIT    = 10000, /* Ignore if pages matched more than this */
    IGNORE_MATCH  = 1000,  /* Ignore if words matched more than this */
    RESULT_MAX = 100,      /* Max number of result displays at once */

    INDEX_MAX = 64,        /* Max number of databases */

    CGI_QUERY_MAX       = 512, /* Max length of a CGI query */
    CGI_INDEX_NAME_MAX  = 64,  /* Max length of an index name in a CGI query */
    CGI_RESULT_NAME_MAX = 64,  /* Max length of a result name in a CGI query */

    SUICIDE_TIME = 60      /* namazu.cgi will suicide when its
			      processing time exceeds this */
};

#endif /* _VALUE_H */
