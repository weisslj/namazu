#ifndef _ENUM_H
#define _ENUM_H

enum {
    ESC = 0x1b,
    BUFSIZE = 1024,

    TOO_MUCH_MATCH = -1,
    TOO_MUCH_HIT = -2,
    REGEX_SEARCH_FAILED = -3,
    PHRASE_SEARCH_FAILED = -4,

    KEY_ITEM_MAX =  16,	 /* Max number of tokens in a query */
    PAGE_MAX = 20,	 /* Max number of result pages */
    IGNORE_HIT = 10000,	 /* Ignore if pages matched more than this */
    IGNORE_MATCH = 1000, /* Ignore if words matched more than this */
    HLIST_MAX_MAX = 100, /* Max number of result displays at once */
    DB_MAX = 64,         /* Max number of databases */
    QUERY_STRING_MAX_LENGTH = 1024, /* Max length of a CGI query */
    DBNAMELENG_MAX = 256,
    QUERY_MAX_LENGTH = 256,

    SUICIDE_TIME = 60,  /* namazu will suicide when its
			   processing time exceeds this */

    SORT_BY_SCORE = 1,  /* at displaying results time */
    SORT_BY_DATE  = 2,  /* at displaying results time */

    EMPHASIZING_START_MARK = '\1',
    EMPHASIZING_END_MARK   = '\2',

    STDIN  = 0,		/* stdin's fd */
    STDOUT = 1,		/* stdout's fd */
    STDERR = 2		/* stderr's fd */

};

#endif /* _ENUM_H */
