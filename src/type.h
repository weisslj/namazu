#ifndef _TYPE_H
#define _TYPE_H

typedef unsigned char uchar;


/* data structure for search result */
struct hlist {
    int n;
    int *scr;  /* score */
    int *fid;  /* file ID */
    int *did;  /* database (index) ID */
    int *date; /* file's date */
};
typedef struct hlist HLIST;

struct replace_elem {
    struct replace_elem *next;
    uchar *src;
    uchar *dst;
    /* The following fields are NULL if this is a traditional
     * string substitution
     */
    struct re_pattern_buffer *src_exp;
    struct subst_elem {
	enum { literal, regex_regno } subst_type;
	union {
	    uchar *literal_string;
	    int regex_regno;
        } u;
    };
};

struct list {
    uchar *str;
    struct list *next;
};
typedef struct list LIST;

struct replace {
    LIST *src;
    LIST *dst;
};
typedef struct replace REPLACE;

struct alias {
    uchar *alias;
    uchar *real;
    struct alias *next;
};
typedef struct alias ALIAS;

#endif /* _TYPE_H */

