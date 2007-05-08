/*
 * $Id: output.c,v 1.119 2007-05-08 03:26:34 opengl2772 Exp $
 * 
 * Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.
 * Copyright (C) 2000-2007 Namazu Project All rights reserved.
 * This is free software with ABSOLUTELY NO WARRANTY.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA
 * 
 * 
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#ifdef HAVE_SUPPORT_H
#  include "support.h"
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include "namazu.h"
#include "libnamazu.h"
#include "codeconv.h"
#include "output.h"
#include "util.h"
#include "field.h"
#include "search.h"
#include "result.h"
#include "i18n.h"
#include "message.h"
#include "form.h"
#include "var.h"
#include "idxname.h"
#include "query.h"
#include "system.h"

static int htmlmode    = 0;   /* 0: text  1: HTML  2: XHTML */
static int cgimode     = 0;
static int quietmode   = 0;

static int countmode   = 0;   /* like grep -c */
static int listmode    = 0;   /* like grep -l */
		      
static int allresult   = 0;   /* print all results */
static int pageindex   = 0;   /* print "Page: [1][2][3][4][5][6][7][8]" */
static int formprint   = 0;   /* print "<form> ... </form>" at cgimode */
static int refprint    = 0;   /* print "References:  [ foo: 123 ]" */
		      
static int maxresult   = 20;  /* max number of search results */
static int listwhence  = 0;   /* number which beginning of search results */
		      
static char template_suffix[BUFSIZE] = "normal"; /* suffix of NMZ.result.* */

/*
 * They are used for emphasizing keywords in html results. 
 */ 
static char emphasis_start_tag[BUFSIZE] = "<strong class=\"keyword\">";
static char emphasis_end_tag[BUFSIZE]   = "</strong>";
static char contenttype[BUFSIZE] = "text/html";

/*
 *
 * Private functions
 *
 */

static void emprint( char *s, int entity_encode );
static void unhtml_buffer( char *ostr );
static void fputs_without_html_tag ( const char *str, FILE *fp );
static char *load_nmz_result(const char *basedir);
static void print_hitnum_each ( struct nmz_hitnumlist *hn );
static int is_allresult ( void );
static int is_pageindex ( void );
static int is_countmode ( void );
static int is_listmode ( void );
static int is_quietmode ( void );
static int is_refprint ( void );
static int is_cgimode ( void );
static enum nmz_stat print_hlist ( NmzResult hlist );
static enum nmz_stat print_listing ( NmzResult hlist );
static void print_query ( const char * qs, int w );
static void print_page_index ( int n );
static void print_current_range ( int listmax );
static void print_hitnum_all_idx ( void );
static void print_hitnum ( int n );
static void print_msgfile ( const char *fname );
static void print_range ( NmzResult hlist );
static void print_errmsg(int errid);

/*
 * Print s to stdout with processing for emphasizing keyword 
 * and entity encoding.
 */
static void 
emprint(char *s, int entity_encode)
{
    int i;
    for (i = 0; i < BUFSIZE * 16 && *s; s++) {
	if (*s == EM_START_MARK) {
	    fputs(emphasis_start_tag, stdout);
	    continue;
	} else if (*s == EM_END_MARK) {
	    fputs(emphasis_end_tag, stdout);
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
            } else if (*s == '\'') {
                fputs("&#39;", stdout);
            } else if (*s == '(') {
                fputs("&#40;", stdout);
            } else if (*s == ')') {
                fputs("&#41;", stdout);
	    } else {
		fputc(*s, stdout);
	    }
	} else {
	    fputc(*s, stdout);
	}
    }
}

/*
 * Output string without HTML elements
 */
static void
unhtml_buffer(char *ostr) {
    int f, i;
    char buf[BUFSIZE] = "", *str;
    int len;

    len = strlen(ostr) + 1;
    str = ostr;
    for (f = 0, i = 0; i < BUFSIZE && *str; str++) {

	if (strncasecmp(str, "<br>", 4) == 0 && *(str + 4) != '\n') {
	    buf[i++] = '\n';
	    str += 3;
	    continue;
        } else if (strncasecmp(str, "<br />", 6) == 0 && *(str + 6) != '\n') {
            buf[i++] = '\n';
            str += 5;
            continue;
        }
	if (*str == '<') {
	    f = 1;
	    continue;
	}
	if (*str == '>') {
	    f = 0;
	    continue;
	}
	if (f == 0) {
	    if (strncmp(str, "&lt;", strlen("&lt;")) == 0) {
		buf[i++] = '<';
		str += 3;
	    } else if (strncmp(str, "&gt;", strlen("&gt;")) == 0) {
		buf[i++] = '>';
		str += 3;
	    } else if (strncmp(str, "&amp;", strlen("&amp;")) == 0) {
		buf[i++] = '&';
		str += 4;
	    } else if (strncmp(str, "&quot;", strlen("&quot;")) == 0) {
		buf[i++] = '"';
		str += 5;
	    } else {
		buf[i++] = *str;
	    }
	}
    }
    buf[i] = '\0';
    strncpy(ostr, buf, len);
    ostr[len - 1] = '\0';
}

static void 
fputs_without_html_tag(const char *str, FILE *fp)
{
    char buf[BUFSIZE] = "";
    strncpy(buf, str, BUFSIZE - 1);
    unhtml_buffer(buf);
    fputs(buf, fp);
}

static void 
print_hitnum_each (struct nmz_hitnumlist *hn)
{
    struct nmz_hitnumlist *hnptr = hn;

    if (hn->phrase != NULL) { /* it has phrases */
	hnptr = hn->phrase;
        if (is_refprint() && !is_countmode() && !is_listmode() && 
	    !is_quietmode())
	{
	    printf(" { ");
	}
    }

    if (is_refprint() && !is_countmode() && 
	!is_listmode() && !is_quietmode()) 
    {
	do {
	    char *converted = nmz_query_external(hnptr->word);
	    if (converted == NULL) {
		die("print_hitnum_each");
	    }

	    printf(" [ ");
	    html_print(converted);
	    free(converted);

	    if (hnptr->stat == SUCCESS) {
		printf(": %d", hnptr->hitnum);
	    } else {
		char *errmsg = nmz_strerror(hnptr->stat);
		printf(" (%s) ", errmsg);
	    }
	    printf(" ] ");
	    hnptr = hnptr->next;
	} while (hn->phrase && hnptr != NULL);
    }

    if (is_refprint() && !is_countmode() && !is_listmode() && 
	!is_quietmode() &&  hn->phrase != NULL) /* it has phrases */
    {
	printf(" :: %d } ", hn->hitnum);
    }
}

static int 
is_allresult(void)
{
    return allresult;
}

static int 
is_pageindex(void)
{
    return pageindex;
}

static int 
is_countmode(void)
{
    return countmode;
}

static int 
is_listmode(void)
{
    return listmode;
}

static int 
is_quietmode(void)
{
    return quietmode;
}

static int 
is_refprint(void)
{
    return refprint;
}

static int 
is_cgimode(void)
{
    return cgimode;

}

static char*
load_nmz_result(const char *basedir)
{
    char fname[BUFSIZE] = "", lang_suffix[BUFSIZE] = "", *buf;
    char templatesuffix[BUFSIZE] = "";

    nmz_pathcat(basedir, NMZ.result);
    strncpy(fname, NMZ.result, BUFSIZE - 1);
    strncat(fname, ".", BUFSIZE - strlen(fname) - 1);
    nmz_delete_since_path_delimitation(templatesuffix, get_templatesuffix(), BUFSIZE);
    strncat(fname, templatesuffix, BUFSIZE - strlen(fname) - 1);  /* usually "normal" */

    if (nmz_choose_msgfile_suffix(fname, lang_suffix) != SUCCESS) {
	nmz_warn_printf("%s: %s", fname, strerror(errno));
	return NULL;
    } 
    strncat(fname, lang_suffix, BUFSIZE - strlen(fname) - 1);

    /* buf is allocated in nmz_readfile. */
    buf = nmz_readfile(fname); 
    if (buf == NULL) { /* failed */
	return NULL;
    }

    return buf;
}

/*
 * Display one searched document according to NMZ.result.* file.
 * e.g.,
 * 
 *   <dt>1. <strong><a href="/foo/gunzip.1.gz">GZIP(1)</a></strong> (score: xx)
 *   <dd><strong>Author</strong>: <em>(unknown)</em>
 *   <dd><strong>Date</strong>: <em>Thu, 09 Apr 1998 12:59:59</em>
 *   <dd>gzip, gunzip, zcat - compress or expand files:: Gzip
 *   reduces the size of the named files using Lempel-Ziv
 *   coding (LZ77). Whenever possible, each file is replaced
 *   by one with the extension .gz, while 
 *   <dd><a href="/foo/gunzip.1.gz">/foo/gunzip.1.gz</a> (5,410 bytes)<br><br>
 */
static enum nmz_stat
print_hlist(NmzResult hlist)
{
    int i, j;
    char *the_template = NULL;    /* User-specified  template. */
    char *template_caches[INDEX_MAX];   /* For caching each NMZ.result */
    char *template = NULL;

    if (hlist.num <= 0 || get_maxresult() == 0) {
	return SUCCESS; /* No document searched but success. */
    }

    /* 
     * Clear pointers for caching NMZ.result.
     */
    for (i = 0; i < nmz_get_idxnum(); i++) {
	template_caches[i] = NULL;
    }

    /* 
     * Check whether user-specified templatedir is set or not. 
     */
    if (is_listmode()) {
        template = "${uri}";
    } else {
	char templdir[BUFSIZE] = "";
	strncpy(templdir, get_templatedir(), BUFSIZE - 1);
        if (*templdir != '\0') { /* user-specified one is set. */
            the_template = load_nmz_result(templdir);
            if (the_template == NULL) {
                return ERR_CANNOT_OPEN_RESULT_FORMAT_FILE;
            }
            if (!is_htmlmode()) {
                char *result = (char *)calloc(sizeof(char), BUFSIZE);
                if (result == NULL) { /* failed */
                    /* Free user-specified template. */
                    if (the_template != NULL) {
                        free(the_template);
                    }
                    return ERR_CANNOT_OPEN_RESULT_FORMAT_FILE;
                }
                strncpy(result, the_template, BUFSIZE - 1);
                free(the_template);
                the_template = result;
                unhtml_buffer(the_template);
            }
            template = the_template;
        }
    }

    for (i = get_listwhence(); i < hlist.num; i++) {
	/* 
	 * Prepare large memory for replace_field() for safety.
	 * FIXME: static memory allocation may cause buffer overflow.
	 */
	char result[BUFSIZE * 128] = "";
	int counter;

	counter = i + 1;

	if (!is_allresult() && (i >= get_listwhence() + get_maxresult())) {
	    break;
        }

	/*
	 * If user-specified templatedir is not set. and 
	 */
	if (!is_listmode() && the_template == NULL) {
            int idxid = hlist.data[i].idxid;
            /* 
             * If NMZ.result is not cached, load NMZ.result and cache it in
             * template_caches[].
             */ 
            if (template_caches[idxid] == NULL) {
                char *basedir = nmz_get_idxname(idxid);
                template_caches[idxid] = load_nmz_result(basedir);
                if (template_caches[idxid] == NULL) {
                    /* Free user-specified template. */
                    if (the_template != NULL) {
                        free(the_template);
                    }
                    /* Free all template_caches[] */
                    for (j = 0; j < nmz_get_idxnum(); j++) {
	                if (template_caches[j] != NULL) {
	                    free(template_caches[j]);
	                }
                    }
                    return ERR_CANNOT_OPEN_RESULT_FORMAT_FILE;
                }
                if (!is_htmlmode()) {
                    char *result = (char *)calloc(sizeof(char), BUFSIZE);
                    if (result == NULL) { /* failed */
                        /* Free user-specified template. */
                        if (the_template != NULL) {
                            free(the_template);
                        }
                        /* Free all template_caches[] */
                        for (j = 0; j < nmz_get_idxnum(); j++) {
	                    if (template_caches[j] != NULL) {
	                        free(template_caches[j]);
	                    }
                        }
                        return ERR_CANNOT_OPEN_RESULT_FORMAT_FILE;
                    }
                    strncpy(result, template_caches[idxid], BUFSIZE - 1);
                    free(template_caches[idxid]);
                    template_caches[idxid] = result;
                    unhtml_buffer(template_caches[idxid]);
                }
            }
            template = template_caches[idxid];
        }

        compose_result(hlist.data[i], counter, template, result);

	{
	    char *converted = nmz_codeconv_external(result);
	    if (converted == NULL) {
		die(nmz_get_dyingmsg());
	    }
	    if (is_htmlmode() || is_listmode()) {
		html_print(converted);
	    } else {
		puts(converted + 1); /* remove '\t' in the head of buffer */
	    }
	    free(converted);
	    printf("\n");
	}
    }

    /* Free user-specified template. */
    if (the_template != NULL) {
	free(the_template);
    }

    /* Free all template_caches[] */
    for (i = 0; i < nmz_get_idxnum(); i++) {
	if (template_caches[i] != NULL) {
	    free(template_caches[i]);
	}
    }

    return SUCCESS;
}

static enum nmz_stat 
print_listing(NmzResult hlist)
{
    enum nmz_stat ret;

    if (is_htmlmode()) {
        printf("<dl>\n");
    }

    ret = print_hlist(hlist);

    if (is_htmlmode()) {
        printf("</dl>\n");
    }

    return ret;
}

/*
 * For page_index().
 */
static void 
print_query(const char * qs, int w)
{
    int foo = 0;
    while (*qs) {
	if (strncmp(qs, "whence=", strlen("whence=")) == 0) {
	    foo = 1;
	    printf("whence=%d", w);
	    for (qs += strlen("whence="); nmz_isdigit((unsigned char)*qs); qs++);
	} else {
	    /* '"' is converted to entities "&quot;" */
	    putc_entitize(*qs);
	    qs++;
	}
    }
    if (foo == 0) {
	printf("&amp;whence=%d", w);
    }
}

/*
 * Display page index.
 */
static void 
print_page_index(int n)
{
    int i, max, whence;
    char *qs; /* QUERY_STRING */
    char *sn; /* SCRIPT_NAME  */
    char *dn; /* DOCUMENT_URI  */

    qs = nmz_getenv("QUERY_STRING");
    sn = nmz_getenv("SCRIPT_NAME");
    dn = nmz_getenv("DOCUMENT_URI");

    if(dn == NULL || *dn == '\0') {
        dn = sn;
    } else {
	/* Delete characters after '?' for Lotus Domino Server R5.06a */
	char *s;
	for (s = dn; *s; s++)
	    if (*s == '?') {*s = 0; break;}
    }

    html_print(_("	<strong>Page:</strong> "));

    max    = get_maxresult();
    whence = get_listwhence();
    for (i = 0; i < PAGE_MAX; i++) {
	if (i * max >= n)
	    break;
	if (is_htmlmode()) {
	    if (i * max != whence) {
		printf("<a href=\"");
		fputs(dn, stdout);
		fputc('?', stdout);
		print_query(qs, i * max);
		printf("\">");
	    } else {
		printf("<strong>");
	    }
	}
	printf("[%d]", i + 1);
	if (is_htmlmode()) {
	    if (i * max != whence) {
		printf("</a> ");
	    } else {
		printf("</strong> ");
	    }
	}
	if (is_allresult()) {
	    break;
	}
    }
}

/*
 * Output current range
 */
static void 
print_current_range(int listmax)
{
    int max, whence;

    max    = get_maxresult();
    whence = get_listwhence();

    if (is_htmlmode()) {
	printf("<strong>");
    }
    printf(_("Current List: %d"), whence + 1);

    printf(" - ");
    if (!is_allresult() && ((whence + max) < listmax)) {
	printf("%d", whence + max);
    } else {
	printf("%d", listmax);
    }
    if (get_htmlmode() == 1) {            /* HTML */
	printf("</strong><br>\n");
    } else if (get_htmlmode() == 2) {     /* XHTML */
	printf("</strong><br />\n");
    } else {
	fputc('\n', stdout);
    }
}

static void 
print_hitnum_all_idx(void)
{
    int idxid;
    for (idxid = 0; idxid < nmz_get_idxnum(); idxid ++) {
        struct nmz_hitnumlist *hnlist = nmz_get_idx_hitnumlist(idxid);

	if (is_refprint() && !is_countmode() && 
	    !is_listmode() && !is_quietmode()) 
	{
	    if (nmz_get_idxnum() > 1) {
	        if (is_htmlmode()) {
		    char *idxname = nmz_get_idxname(idxid);
		    if (is_cgimode()) {
			/* For hiding a full pathname of an index */
			idxname =  nmz_get_idxname(idxid) 
			    + strlen(nmz_get_defaultidx()) + 1;
		    }
		    printf("<li><strong>");
		    puts_entitize(idxname);
		    printf("</strong>: ");
		} else {
		    printf("(%s)", nmz_get_idxname(idxid));
		}
	    }
	}

        while (hnlist != NULL) {
	    print_hitnum_each(hnlist);
	    hnlist = hnlist->next;
	}

	if (is_refprint() && !is_countmode() && !is_listmode() && 
	    !is_quietmode()) {
	    if (nmz_get_idxnum() > 1 && nmz_get_querytokennum() > 1) {
	        printf(_(" [ TOTAL: %d ]"), nmz_get_idx_totalhitnum(idxid));
	    }
	    if (nmz_get_idxnum() > 1) {
                if (is_htmlmode()) {
		    printf("</li>");
                }
            }
	    printf("\n");
	}
    }
}

static void 
print_hitnum(int n)
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

/* 
 * Output contents of a message file such as NMZ.tips or NMZ.body. 
 */
static void 
print_msgfile(const char *fname) {
    char suffix[BUFSIZE] = "", tmpfname[BUFSIZE] = "";

    if (nmz_choose_msgfile_suffix(fname, suffix) == SUCCESS) {
	char *buf;

	strncpy(tmpfname, fname, BUFSIZE - 1);
	strncat(tmpfname, suffix, BUFSIZE - strlen(tmpfname) - 1);

	buf = nmz_readfile(tmpfname); /* buf is allocated in nmz_readfile. */
	if (buf == NULL) {
	    die(nmz_get_dyingmsg());
	}
	/* In case of suffix isn't equal to lang, we need code conversion */
	if (strcmp(suffix, nmz_get_lang()) != 0) {
            /* new is allocated in nmz_codeconv_external. */
	    char *new = nmz_codeconv_external(buf); 
	    free(buf);  /* Then we should free buf's memory */
	    buf = new;
	}

	fputs(buf, stdout);
	free(buf);
    } else {
	nmz_warn_printf("%s: %s", fname, strerror(errno));
    }
}

static void 
print_range(NmzResult hlist)
{
    if (is_htmlmode()) {
        printf("<div class=\"namazu-result-footer\">\n");
    }
    print_current_range(hlist.num);
    if (is_pageindex()) {
        print_page_index(hlist.num);
    }
    if (is_htmlmode()) {
        printf("</div>\n");  /* class="namazu-result-footer" */
    } else {
        printf("\n");
    }
}

/*
 * Print the error message specified by errid while outputing the results.
 */
static void
print_errmsg(int errid)
{
    char *errmsg = nmz_strerror(errid);
    char buf[BUFSIZE] = "";
    snprintf(buf, BUFSIZE - 1, _("	<h2>Error!</h2>\n<p>%s</p>\n"), errmsg);
    html_print(buf);
}

/*
 *
 * Public functions
 *
 */
enum nmz_stat 
print_result(NmzResult hlist, const char *query, const char *subquery)
{
    if (is_htmlmode()) {
        if (is_cgimode()) {
            printf("%s %s" CRLF CRLF, MSG_MIME_HEADER, get_contenttype());
        }
	print_headfoot(NMZ.head, query, subquery);
    }

    if (hlist.stat != SUCCESS) {
	print_errmsg(hlist.stat);
	return FAILURE;
    }

    if (!is_countmode() && !is_listmode() && !is_quietmode()) {
        if (is_htmlmode()) {
            fputs("<div class=\"namazu-result-header\">\n", stdout);
        }
    }

    /* Result1:  <h2>Results:</h2>, References:  */
    if (is_refprint() && !is_countmode() && 
	!is_listmode() && !is_quietmode()) 
    {
	html_print(_("	<h2>Results:</h2>\n"));

	if (is_htmlmode()) {
	    fputs("<p>\n", stdout);
	} else {
	    fputc('\n', stdout);
	}
	printf(_("References: "));
	if (nmz_get_idxnum() > 1 && is_htmlmode()) {
	    fputs("</p>\n", stdout);
	}

        if (nmz_get_idxnum() > 1) {
            printf("\n");
            if (is_htmlmode())
                printf("<ul>\n");
        }
    }

    print_hitnum_all_idx(); /* print hit numbers for all index. */

    if (is_refprint() && !is_countmode() && 
	!is_listmode() && !is_quietmode()) {
        if (nmz_get_idxnum() > 1 && is_htmlmode()) {
            printf("</ul>\n");
        }
	if (nmz_get_idxnum() == 1 && is_htmlmode()) {
	    printf("\n</p>\n");
	} else {
	    fputc('\n', stdout);
	}
    }

    if (hlist.num > 0) {
        if (!is_countmode() && !is_listmode() && !is_quietmode()) {
            print_hitnum(hlist.num);  /* <!-- HIT -->%d<!-- HIT --> */
	    if (is_htmlmode()) {
	        fputs("</div>\n", stdout); /* class="namazu-result-header" */
            }
        }
	if (is_countmode()) {
	    printf("%d\n", hlist.num);
	} else {
	    enum nmz_stat ret = print_listing(hlist);

	    if (ret != SUCCESS) { /* summary listing */
		print_errmsg(ret);
		return ret;
            }
	}
        if (!is_countmode() && !is_listmode() && !is_quietmode()) {
            print_range(hlist);
        }
    } else {
        if (is_countmode()) {
            printf("0\n");
        } else if (!is_listmode() && !is_quietmode()) {
	    html_print(_("	<p>No document matching your query.</p>\n"));
            if (is_htmlmode()) {
	        fputs("</div>\n", stdout); /* class="namazu-result-header" */
		print_msgfile(NMZ.tips);
	    }
        }
    }

    if (is_htmlmode()) {
	print_headfoot(NMZ.foot, query, subquery);
    }

    return SUCCESS;
}

/* 
 * Print default page: NMZ.{head,body,foot} 
 */
void 
print_default_page (void) {
    if (is_htmlmode()) {
        if (is_cgimode()) {
	    printf("%s %s" CRLF CRLF, MSG_MIME_HEADER, get_contenttype());
        }
	print_headfoot(NMZ.head, "", "");
	print_msgfile(NMZ.body);
	print_headfoot(NMZ.foot, "", "");
    }
}

/*
 * output string as non-html (replace special characters by entities)
 */
void
puts_entitize(const char *str)
{
    const char *p = str;
    for (; *p != 0; p++) {
	putc_entitize(*p);
    }
}

void
putc_entitize(int c)
{
	if (c == '<') {
	    fputs("&lt;", stdout);
	} else if (c == '>') {
	    fputs("&gt;", stdout);
	} else if (c == '&') {
	    fputs("&amp;", stdout);
	} else if (c == '"') {
	    fputs("&quot;", stdout);
        } else if (c == '\'') {
            /* not support &apos; for HTML 4.01 */
            /* fputs("&apos;", stdout); */
            fputs("&#39;", stdout);
        } else if (c == '(') {
            fputs("&#40;", stdout);
        } else if (c == ')') {
            fputs("&#41;", stdout);
	} else {
	    fputc(c, stdout);
	}
}

void 
set_emphasis_tags(const char *start_tag, const char *end_tag)
{
    strncpy(emphasis_start_tag, start_tag, BUFSIZE - 1);
    strncpy(emphasis_end_tag,   end_tag, BUFSIZE - 1);
}

char *
get_emphasis_tag_start(void)
{
    return emphasis_start_tag;
}

char *
get_emphasis_tag_end(void)
{
    return emphasis_end_tag;
}

void
set_contenttype(const char *str)
{
    strncpy(contenttype, str, BUFSIZE - 1);
}

char *
get_charset_str(void)
{
    static char buff[BUFSIZE] = "";
    char *charset;

    if ((charset = nmz_get_external_encoding()) == NULL) {
        charset = "UTF-8";
    }
    sprintf(buff, "charset=%s", charset);
    return(buff);
}

char *
get_contenttype(void)
{
    static char buff[BUFSIZE];
    char *p;
    int is_charset = 0;


    buff[0] = '\0';
    p = strtok(contenttype, " \t\n\r;");
    while(p) {
        if (!strncasecmp(p, "charset=", strlen("charset="))) {
            is_charset = 1;
            break;
        }
        p = strtok(NULL, " \t\n\r;");
    }

    if (!is_charset) {
        sprintf(buff, "%s; %s", contenttype, get_charset_str());
        return(buff);
    }

    return(contenttype);
}

void 
set_htmlmode(int mode)
{
    htmlmode = mode;
}

int
get_htmlmode(void)
{
    return htmlmode;
}

int
is_htmlmode(void)
{
    return htmlmode != 0 ? 1 : 0;
}

void 
set_cgimode(int mode)
{
    cgimode = mode;
}

void 
set_quietmode(int mode)
{
    quietmode = mode;
}

void 
set_countmode(int mode)
{
    countmode = mode;
}

void 
set_listmode(int mode)
{
    listmode = mode;
}

void 
set_allresult(int mode)
{
    allresult = mode;
}

void 
set_pageindex(int mode)
{
    pageindex = mode;
}

void 
set_formprint(int mode)
{
    formprint = mode;
}

int 
is_formprint(void)
{
    return formprint;
}

void 
set_refprint(int mode)
{
    refprint = mode;
}

void 
set_maxresult(int num)
{
    maxresult = num;
}

int 
get_maxresult(void)
{
    return maxresult;
}

void 
set_listwhence(int num)
{
    listwhence = num;
}

int 
get_listwhence(void)
{
    return listwhence;
}

void 
set_templatesuffix(const char *tmpl)
{
    int i = 0;
    char *p;

    strncpy(template_suffix, tmpl, BUFSIZE - 1);
    template_suffix[BUFSIZE - 1] = '\0';

    /*
     *   result : [A-Za-z][A-Za-z0-9_\-]*
     *   (ex. normal, short)
     */
    p = template_suffix;
    while(*p) {
        if (!((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') ||
        (((*p >= '0' && *p <= '9') || *p == '_' || *p == '-')&& i != 0))) {
            *p = '\0';
            break;
        }
        p++;
        i++;
    }

    if (template_suffix[0] == '\0') {
        strncpy(template_suffix, "normal", BUFSIZE - 1);
        template_suffix[BUFSIZE - 1] = '\0';
    }
}

char *
get_templatesuffix(void)
{
    return template_suffix;
}


/* 
 * Namazu version fputs, it works with considereation of html mode.
 */
void 
html_print(const char *str)
{
    char buf[BUFSIZE * 16] = "";
    int is_nmz_html = 0;

    if ((int)*str == (int)'\t') { /* Namazu's HTML message */
        is_nmz_html = 1;
    }

    strcpy(buf, str + is_nmz_html);
    if (is_htmlmode()) {
        /* 
	 * If str is Namazu's HTML message, it will be printed with emprint.
	 * If not, it will be printed with entity conversion 
	 */
        emprint(buf, ! is_nmz_html);
    } else {
        /* 
	 * If str is Namazu's HTML message, it will be printed without 
	 * HTML tag, if not, it will be printed as is 
	 */
        if (is_nmz_html) {
            fputs_without_html_tag(buf, stdout);
        } 
	else {
            fputs(buf, stdout);
        }
    }
}


/* 
 * Print the error message and die.
 */
void 
die(const char *fmt, ...)
{
    va_list args;

    fflush(stdout);
    fflush(stderr);

    if (is_htmlmode()) {
        static char msg[BUFSIZE] = "";

        va_start(args, fmt);
        vsnprintf(msg, BUFSIZE - 1, fmt, args);
        va_end(args);

        if (is_cgimode()) {
            /* FIXME: Use nmz_codeconv_external. */
            printf("%s %s; %s" CRLF CRLF, MSG_MIME_HEADER,
                "text/html", "charset=UTF-8");
        }
        printf("<html><body>\n");
        printf(_("<h2>Error</h2>\n<p>"));
        puts_entitize(msg);
        printf(_("</p>"));
        printf("\n</body></html>\n");
    } else {
	fprintf(stderr, "%s: ", PACKAGE);
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
        if (fmt[strlen(fmt) - 1] != '\n') {
            fprintf(stderr, "\n");
        }
    }

    exit_nmz(EXIT_FAILURE);
}

