#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "namazu.h"
#include "libnamazu.h"
#include "codeconv.h"
#include "output.h"
#include "util.h"
#include "field.h"
#include "search.h"
#include "result.h"
#include "em.h"
#include "i18n.h"
#include "var.h"
#include "magic.h"
#include "mode.h"

static int htmlmode    = 0;
static int cgimode     = 0;
static int quietmode   = 0;

static int countmode   = 0;   /* like grep -c */
static int listmode    = 0;   /* like grep -l */
		      
static int allresult   = 0;   /* print all results */
static int pageindex   = 0;   /* print "Page: [1][2][3][4][5][6][7][8]" */
static int formprint   = 1;   /* print "<form> ... </form>" at cgimode */
static int refprint    = 1;   /* print "References:  [ foo: 123 ]" */
		      
static int urireplace  = 1;   /* replace URI in results */
static int uridecode   = 0;   /* decode URI in results */

static int maxresult = 20;  /* max number of search results */
static int listwhence  = 0;   /* number which beginning of search results */
		      
static char template[BUFSIZE]     = "normal"; /* suffix of NMZ.result.* */

/*
 *
 * Private functions
 *
 */

static void fputs_without_html_tag(char* , FILE*);
static void emprint(char*, int);
static void print_word_hit_count (char *key, int hitnum);

/* print s to stdout with processing for emphasizing and entity encoding  */
static void emprint(char *s, int entity_encode)
{
    int i;
    for (i = 0; *s && i < BUFSIZE; s++) {
	/* iso-2022-jp handling */
	if ((strncmp(s, "\033$", 2) == 0)
	    && (*(s + 2) == 'B' || *(s + 2) == '@')) 
	{
	    char *p;
	    int n = 0;
	    char buf[BUFSIZE];

	    strncpy(buf, s, 3);
	    n += 3;
	    s += 3;
	    p = strstr(s, "\033(");
	    if (p == NULL) {   /* non-terminating jis x 0208 */
		fputs(s, stdout);
		return; 
	    }
	    if (*(p + 2) == 'J' || *(p + 2) == 'B' || *(p + 2) == 'H') {
		int len = p - s + 3;
		strncpy(buf + n, s, len);
		n += len;
		buf[n] = '\0';
		fputs(buf, stdout);
		s += len;
	    } else {  /* unknown charset designation */
		fputs(s, stdout);
		return;
	    }
	}
	if (*s == EM_START_MARK) {
	    fputs(EM_START_TAG, stdout);
	    continue;
	} else if (*s == EM_END_MARK) {
	    fputs(EM_END_TAG, stdout);
	    continue;
	} 
	if (entity_encode) {
	    /* < > & " are converted to entities like &lt; */
	    if (*s == '<') {
		fputs("&lt;", stdout);
	    } else if (*s == '>') {
		fputs("&gt;", stdout);
	    } else if (*s == '&') {
		fputs("&amp;", stdout);
	    } else if (*s == '"') {
		fputs("&quot;", stdout);
	    } else {
		fputc(*s, stdout);
	    }
	} else {
	    fputc(*s, stdout);
	}
    }
}

/* output string without HTML elements */
static void fputs_without_html_tag(char * s, FILE *fp)
{
    int f, i;
    char buf[BUFSIZE];

    for (f = 0, i = 0; *s && i < BUFSIZE; s++) {

	/* iso-2022-jp handling */
	if ((strncmp(s, "\033$", 2) == 0)
	    && (*(s + 2) == 'B' || *(s + 2) == '@')) 
	{
	    char *p;

	    strncpy(buf + i, s, 3);
	    i += 3;
	    s += 3;
	    p = strstr(s, "\033(");
	    if (p == NULL) {   /* non-terminating jis x 0208 */
		strcpy(buf + i, s);
		return; 
	    }
	    if (*(p + 2) == 'J' || *(p + 2) == 'B' || *(p + 2) == 'H') {
		int len = p - s + 3;
		strncpy(buf + i, s, len);
		i += len;
		s += len;
	    } else {  /* unknown charset designation */
		strcpy(buf + i, s);
		return;
	    }
	}

	if (strncasecmp(s, "<br>", 4) == 0&& *(s + 4) != '\n') {
	    buf[i++] = '\n';
	    s += 3;
	    continue;
	}
	if (*s == '<') {
	    f = 1;
	    continue;
	}
	if (*s == '>') {
	    f = 0;
	    continue;
	}
	if (f == 0) {
	    if (strncmp(s, "&lt;", 4) == 0) {
		buf[i++] = '<';
		s += 3;
	    } else if (strncmp(s, "&gt;", 4) == 0) {
		buf[i++] = '>';
		s += 3;
	    } else if (strncmp(s, "&amp;", 5) == 0) {
		buf[i++] = '&';
		s += 4;
	    } else {
		buf[i++] = *s;
	    }
	}
    }
    buf[i] = '\0';
    fputs(buf, fp);
}



/*
 *
 * Public functions
 *
 */

void set_htmlmode(int mode)
{
    htmlmode = mode;
}

int is_htmlmode(void)
{
    return htmlmode;
}

void set_cgimode(int mode)
{
    cgimode = mode;
}

int is_cgimode(void)
{
    return cgimode;
}

void set_quietmode(int mode)
{
    quietmode = mode;
}

int is_quietmode(void)
{
    return quietmode;
}

int is_countmode(void)
{
    return countmode;
}

void set_countmode(int mode)
{
    countmode = mode;
}

void set_listmode(int mode)
{
    listmode = mode;
}

int is_listmode(void)
{
    return listmode;
}

void set_allresult(int mode)
{
    allresult = mode;
}

int is_allresult(void)
{
    return allresult;
}

void set_pageindex(int mode)
{
    pageindex = mode;
}

int is_pageindex(void)
{
    return pageindex;
}

void set_formprint(int mode)
{
    formprint = mode;
}

int is_formprint(void)
{
    return formprint;
}

void set_refprint(int mode)
{
    refprint = mode;
}

int is_refprint(void)
{
    return refprint;
}

void set_urireplace(int mode)
{
    urireplace = mode;
}

int is_urireplace(void)
{
    return urireplace;
}

void set_uridecode(int mode)
{
    uridecode = mode;
}

int is_uridecode(void)
{
    return uridecode;
}

void set_maxresult(int num)
{
    maxresult = num;
}

int get_maxresult(void)
{
    return maxresult;
}

void set_listwhence(int num)
{
    listwhence = num;
}

int get_listwhence(void)
{
    return listwhence;
}


void set_template(char *tmpl)
{
    strcpy(template, tmpl);
}

char *get_template(void)
{
    return template;
}

/*
 * for pageindex
 */
void put_query(char * qs, int w)
{
    int foo = 0;
    while (*qs) {
	if (strprefixcmp(qs, "whence=") == 0) {
	    foo = 1;
	    printf("whence=%d", w);
	    for (qs += strlen("whence="); isdigit(*qs); qs++);
	} else {
	    fputc(*(qs++), stdout);
	}
    }
    if (foo == 0) {
	printf("&whence=%d", w);
    }
}


/* displayin page index */
void put_page_index(int n)
{
    int i, max, whence;
    char *qs; /* QUERY_STRING */
    char *sn; /* SCRIPT_NAME  */

    qs = safe_getenv("QUERY_STRING");
    sn = safe_getenv("SCRIPT_NAME");

    html_print(_("	<strong>Page:</strong> "));

    max    = get_maxresult();
    whence = get_listwhence();
    for (i = 0; i < PAGE_MAX; i++) {
	if (i * max >= n)
	    break;
	if (is_htmlmode()) {
	    if (i * max != whence) {
		print("<a href=\"");
		fputs(sn, stdout);
		fputc('?', stdout);
		put_query(qs, i * max);
		print("\">");
	    } else {
		print("<strong>");
	    }
	}
	printf("[%d]", i + 1);
	if (is_htmlmode()) {
	    if (i * max != whence) {
		print("</A> ");
	    } else
		print("</strong> ");
	}
	if (is_allresult()) {
	    break;
	}
    }
}

/* output current range */
void put_current_range(int listmax)
{
    int max, whence;

    max    = get_maxresult();
    whence = get_listwhence();

    if (is_htmlmode()) {
	print("<strong>");
    }
    printf(_("Current List: %d"), whence + 1);

    print(" - ");
    if (!is_allresult() && ((whence + max) < listmax)) {
	printf("%d", whence + max);
    } else {
	printf("%d", listmax);
    }
    if (is_htmlmode()) {
	print("</strong><br>\n");
    } else {
	fputc('\n', stdout);
    }
}

/* fputs Namazu version, it works with considereation of mode */
void html_print(char *str)
{
    char buf[BUFSIZE * 16];
    int is_nmz_html = 0;

    if ((int)*str == (int)'\t') { /* Namazu's HTML message */
        is_nmz_html = 1;
    }

    strcpy(buf, str + is_nmz_html);
    if (is_htmlmode()) {
        /* if str is Namazu's HTML message, it will be printed with emprint,
           if not, it will be printed with entity conversion */
        emprint(buf, ! is_nmz_html);
    } else {
        /* if str is Namazu's HTML message, it will be printed without 
           HTML tag, if not, it will be printed as is */
        if (is_nmz_html) {
            fputs_without_html_tag(buf, stdout);
        } 
	else {
            fputs(buf, stdout);
        }
    }
}
/*
 * display message file such as NMZ.tips or NMZ.body.
 */

void print_msgfile(char *fname) {
    char tmp_fname[BUFSIZE];

    strcpy(tmp_fname, fname);
    choose_msgfile(tmp_fname);
    cat(tmp_fname);
}

/* display the hlist */
void print_hlist(HLIST hlist)
{
    int i;
    char *templates[INDEX_MAX];
    /* prepare large memory for replace_field() and conv_ext() */
    char result[BUFSIZE * 128];

    if (hlist.n <= 0 || get_maxresult() == 0) {
	return;
    }

    /* set NULL to all templates[] */
    for (i = 0; i < Idx.num; i++) {
	templates[i] = NULL;
    }

    for (i = get_listwhence(); i < hlist.n; i++) {
	int counter;

	counter = i + 1;

	if (!is_allresult() && (i >= get_listwhence() + get_maxresult()))
	    break;
	if (is_listmode()) {
	    get_field_data(hlist.d[i].idxid, hlist.d[i].docid, "uri", result);
	} else {
	    if (templates[hlist.d[i].idxid] == NULL) {  /* not loaded */
		char fname[BUFSIZE];

		make_fullpathname_result(hlist.d[i].idxid);
		strcpy(fname, NMZ.result);
		strcat(fname, ".");
		strcat(fname, get_template());  /* usually "normal" */
		templates[hlist.d[i].idxid] = readfile(fname);
	    }
	    compose_result(hlist.d[i], counter, 
			   templates[hlist.d[i].idxid],  result);
	}
	conv_ext(result);
	html_print(result);
	print("\n");
    }

    /* free all templates[] */
    for (i = 0; i < Idx.num; i++) {
	if (templates[i] != NULL) {
	    free(templates[i]);
	}
    }
}

void print_hit_count ()
{
    int i;
    for (i = 0; i < Idx.num; i ++) {
        PHRASERES *pr = Idx.pr[i];
	if (is_refprint() && !is_countmode() && 
	    !is_listmode() && !is_quietmode()) 
	{
	    if (Idx.num > 1) {
	        if (is_htmlmode()) {
		    printf("<li><strong>%s</strong>: ",
			   Idx.names[i] + strlen(DEFAULT_INDEX) + 1);
		} else {
		    printf("(%s)", Idx.names[i]);
		}
	    }
	}
        if (is_refprint() && !is_countmode() && !is_listmode() && 
	    !is_quietmode() && Idx.mode[i] == PHRASE_MODE) 
	{
	    print(" { ");
	}
        while (pr != NULL) {
	    print_word_hit_count(pr->word, pr->hitnum);
	    pr = pr->next;
	}
	if (is_refprint() && !is_countmode() && !is_listmode() && 
	    !is_quietmode() && Idx.mode[i] == PHRASE_MODE) 
	{
	    printf(" :: %d } ", Idx.phrasehit[i]);
	}
	if (is_refprint() && !is_countmode() && !is_listmode() && 
	    !is_quietmode()) {
	    if (Idx.num > 1 && Query.tab[1]) {
	        printf(_(" [ TOTAL: %d ]"), Idx.total[i]);
	    }
	    print("\n");
	}
    }
}

static void print_word_hit_count (char *key, int hitnum)
{
    if (is_refprint() && !is_countmode() && 
	!is_listmode() && !is_quietmode()) {
	char tmp_key[BUFSIZE];
	strcpy(tmp_key, key);
	conv_ext(tmp_key);

        print(" [ ");
        print(tmp_key);
        if (hitnum > 0) {
            printf(": %d", hitnum);
        } else { 
            char *msg = (char *)"";
            if (hitnum == 0) {
                msg = (char *)": 0 ";
            } else if (hitnum == ERR_TOO_MUCH_MATCH) {
                msg = _(" (Too many words matched. Ignored.)");
            } else if (hitnum == ERR_TOO_MUCH_HIT) {
                msg = _(" (Too many pages hit. Ignored.)");
            } else if (hitnum == ERR_REGEX_SEARCH_FAILED) {
                msg = _(" (cannot open regex index)");
            } else if (hitnum == ERR_PHRASE_SEARCH_FAILED) {
                msg = _(" (cannot open phrase index)");
            } else if (hitnum == ERR_CANNOT_OPEN_INDEX) {
		msg = _(" (cannot open this index)\n");
            } else if (hitnum == ERR_NO_PERMISSION) {
		msg = _("(You don\'t have a permission to access the index)");
	    }
            print(_(msg));
        }
        print(" ] ");
    }
}

void print_result1(void)
{
    html_print(_("	<h2>Results:</h2>\n"));

    if (is_htmlmode()) {
	fputs("<p>\n", stdout);
    } else {
	fputc('\n', stdout);
    }
    print(_("References: "));
    if (Idx.num > 1 && is_htmlmode()) {
	fputs("</p>\n", stdout);
    }
}

void print_hitnum(int n)
{
    html_print(_("	<p><strong> Total "));
    if (is_htmlmode()) {
        printf("<!-- HIT -->%d<!-- HIT -->", n);
    }
    else {
        printf("%d", n);
    }
    html_print(_("	 documents matching your query.</strong></p>\n\n"));
}

void print_listing(HLIST hlist)
{
    if (is_htmlmode()) {
        print("<dl>\n");
    }
    
    print_hlist(hlist);
    
    if (is_htmlmode()) {
        print("</dl>\n");
    }
}

void print_range(HLIST hlist)
{
    if (is_htmlmode())
        print("<p>\n");
    put_current_range(hlist.n);
    if (is_pageindex()) {
        put_page_index(hlist.n);
    }
    if (is_htmlmode()) {
        print("</p>\n");
    } else {
        print("\n");
    }
}

