#ifndef _NAMAZU_H
#define _NAMAZU_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <time.h>

#define BUFSIZE 1024

typedef unsigned char uchar;	/* unsigned char to uchar */

#define ESC 0x1b
#define ASCII "\x1b(B"
#define NEWJIS_K "\x1b$B"
#define iseuc(c)  ((c) >= 0xa1 && (c) <= 0xfe)
#define set1byte() fputs( ASCII, stdout )
#define set2byte() fputs( NEWJIS_K, stdout )


#ifdef __EMX__
#define Chdir(a) _chdir2(a)
#define Getcwd(a,b) _getcwd2(a,b)
#else
#define Chdir(a) chdir(a)
#define Getcwd(a,b) getcwd(a,b)
#endif

#define is_lang_ja(a) (!strcmp(a,"ja"))

enum {
    TOO_MUCH_MATCH = -1,
    TOO_MUCH_HIT = -2,
    REGEX_SEARCH_FAILED = -3,
    PHRASE_SEARCH_FAILED = -4,

    KEY_ITEM_MAX =  16,	 /* Max number of tokens in a query */
    PAGE_MAX = 20,	 /* Max number of result pages */
    IGNORE_HIT = 10000,	 /* Igreno if pages matched more than this */
    IGNORE_MATCH = 1000, /* Ignore if words matched more than this */
    HLIST_MAX_MAX = 100, /* Max number of result displays at once */
    DB_MAX = 64,         /* Max number of databases */
    QUERY_STRING_MAX_LENGTH = 1024, /* Max length of a CGI query */
    DBNAMELENG_MAX = 256,
    QUERY_MAX_LENGTH = 256,

    STDIN  = 0,		/* stdin's fd */
    STDOUT = 1,		/* stdout's fd */
    STDERR = 2		/* stderr's fd */
};

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


/* messages */
extern uchar *MSG_USAGE, *MSG_TOO_LONG_KEY, *MSG_TOO_MANY_KEYITEM,
*MSG_QUERY_STRING_TOO_LONG, *MSG_RESULT_HEADER, *MSG_NO_HIT, *MSG_HIT_1, 
*MSG_HIT_2, *MSG_TOO_MUCH_HIT, *MSG_TOO_MUCH_MATCH, *MSG_INDEXDIR_ERROR,
*MSG_REFERENCE_HEADER, *MSG_INVALID_DB_NAME, *MSG_INVALID_QUERY,
*MSG_CANNOT_OPEN_INDEX, *MSG_MIME_HEADER, *MSG_CANNOT_OPEN_REGEX_INDEX,
*MSG_CANNOT_OPEN_PHRASE_INDEX, *MSG_CANNOT_OPEN_FIELD_INDEX;

/* extern uchar *VERSION; */
extern uchar *COPYRIGHT;

/* global variables */
extern int HListMax;
extern int HListWhence;
extern int Debug;
extern int ShortFormat;
extern int MoreShortFormat;
extern int Quiet;
extern int HitCountOnly;
extern int ScoreSort;
extern int HtmlOutput;
extern int HidePageIndex;
extern int ForcePrintForm;
extern int AllList;
extern int LaterOrder;
extern int FinalHitN;
extern int ConfLoaded;
extern int NoReplace;
extern int DecodeURI;
extern int IsCGI;
extern int Logging;
extern int DbNumber;
extern int OppositeEndian;
extern int AllDocumentN;
extern int TfIdf;
extern int NoReference;

extern uchar KeyTable[];
extern uchar DbName[];
extern uchar *KeyItem[];
extern uchar DEFAULT_DIR[];
extern uchar *DbNames[];
extern uchar Lang[];
extern FILE *Index, *IndexIndex;

extern uchar BASE_URI[];
extern uchar NAMAZU_CONF[];
extern uchar NAMAZURC[];

extern uchar *ScriptName;
extern uchar *QueryString;
extern uchar *ContentLength;

extern uchar INDEX[];
extern uchar INDEXINDEX[];
extern uchar HEADERFILE[];
extern uchar FOOTERFILE[];
extern uchar LOCKFILE[];
extern uchar LOCKMSGFILE[];
extern uchar BODYMSGFILE[];
extern uchar RESULTFILE[];
extern uchar SLOG[];
extern uchar WORDLIST[];
extern uchar FIELDINFO[];
extern uchar DATEINDEX[];

extern uchar PHRASE[];
extern uchar PHRASEINDEX[];

extern REPLACE Replace;
extern ALIAS   *Alias;

/* functions */
uchar jmstojis(uchar, uchar);
void jistoeuc();
void sjistoeuc();
void euctojisput();
uchar jistojms(uchar, uchar);
void euctosjis();
void error();
int ismetastring();
void fputs_without_html_tag();
void fputs_with_codeconv();
void put_body_msg();
void cat_head_or_foot();
long get_index_pointer();
int binsearch();
void malloc_hlist();
void realloc_hlist();
void memcpy_hlist();
HLIST merge_hlist();
void set_did_hlist();
void free_hlist();
void copy_hlist();
HLIST get_hlist();
HLIST do_date_processing();
void nmz_mergesort();
void sort_hlist();
uchar URIdecode(uchar, uchar);
void decode_uri_string();
void put_hlist();
void show_usage();
HLIST andmerge();
HLIST notmerge();
HLIST ormerge();
HLIST factor();
int andop();
HLIST term();
HLIST regex_grep();
int orop();
HLIST expr();
void reverse_hlist();
void initialize_parser();
void queryput();
void put_page_index();
void put_current_extent();
void search_main();
int codeconv();
int get_cgi_variables();
void zen2han();
void show_configuration();
void load_namazu_conf();
void cgi_initialize();
void initialize_message();
void cat();
void reverse_byte_order();
void set_pathname();
void pathcat();
HLIST do_search();
int main();
int open_index_files();
void close_index_files();
void lrget();
void wakati();
void split_query();
int is_field();
void fputx();
void codeconv_query();
int replace_uri();
void apply_field_alias();

#endif /* _NAMAZU_H */
