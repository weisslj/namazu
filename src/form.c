/*
 * 
 * form.c -
 * 
 * $Id: form.c,v 1.81 2005-11-02 08:47:43 opengl2772 Exp $
 * 
 * Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.
 * Copyright (C) 2000-2005 Namazu Project All rights reserved.
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
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#ifdef HAVE_SUPPORT_H
#  include "support.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#else
# ifdef _WIN32
# include <io.h>
# endif
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include "namazu.h"
#include "libnamazu.h"
#include "util.h"
#include "form.h"
#include "output.h"
#include "field.h"
#include "hlist.h"
#include "codeconv.h"
#include "i18n.h"
#include "var.h"
#include "idxname.h"


/*
 *
 * Private functions
 *
 */

static int cmp_element ( const char *s1, const char *s2 );
static enum nmz_stat replace_query_value ( const char *p, const char *query );
static void delete_str ( char *s, char *d );
static void get_value ( const char *s, char *value );
static void get_select_name ( const char *s, char *value );
static enum nmz_stat select_option ( char *s, const char *name, const char *query, const char *subquery );
static enum nmz_stat check_checkbox ( char *str, const char *query );
static void handle_tag ( const char *start, const char *end, const char *query,char *select_name, const char *subquery );
static char * read_headfoot ( const char *fname );
static void subst ( char *str, const char *pat, const char *rep );

/* 
 * Compare given two elements
 * some measure of containing LF or redundant spaces are acceptable.
 * igonore cases.
 */
static int 
cmp_element(const char *s1, const char *s2)
{
    for (; *s1 && *s2; s1++, s2++) {
        if (*s2 == ' ') {
            while (*s1 == ' ' || *s1 == '\t' || *s1 == '\n' || *s1 == '\r') {
                s1++;
            }
            s2++;
        }
        if (_nmz_toupper((unsigned char)*s1) != _nmz_toupper((unsigned char)*s2)) {
            break;
        }
    }
    if (*s2 == '\0') {
        return 0;
    } else {
        return 1;
    }
}

/* 
 * replace <input type="text" name="query"  value="hogehoge"> 
 */
static enum nmz_stat
replace_query_value(const char *p, const char *query)
{
    if (cmp_element(p, (char *)"input type=\"text\" name=\"query\"") == 0) {
	char *converted;

        {
            char buffer[BUFSIZE] = "";
            char value[BUFSIZE] = "";

            get_value(p, value);

            strncpy(buffer, " value=\"", BUFSIZE - 1);
            strncat(buffer, value, BUFSIZE - strlen(buffer) - 1);
            strncat(buffer, "\"", BUFSIZE - strlen(buffer) - 1);
            buffer[BUFSIZE - 1] = '\0';
            delete_str((char *)p, buffer);

            strncpy(buffer, " value='", BUFSIZE - 1);
            strncat(buffer, value, BUFSIZE - strlen(buffer) - 1);
            strncat(buffer, "'", BUFSIZE - strlen(buffer) - 1);
            buffer[BUFSIZE - 1] = '\0';
            delete_str((char *)p, buffer);
	}
        converted = nmz_query_external(query);
	if (converted == NULL) {
	    die("%s", strerror(errno));
	}

        for (; *p; p++)
            fputc(*p, stdout);

        {
            printf(" value=\"");
            html_print(converted);  /* for treating <>&" chars in the query. */
            printf("\"");
        }
	free(converted);
        return SUCCESS;
    }
    return FAILURE;
}

char *
nmz_query_external(const char *query)
{
    char *query_external;
    char *alloc;
    char *pos, *p;

    alloc = strdup(query);
    if (alloc == NULL) { /* */
        nmz_set_dyingmsg(nmz_msg("%s", strerror(errno)));
        return NULL;
    }

    nmz_codeconv_internal(alloc);

    p = alloc;
    while(*p == '\t') {
        p++;
    }
    pos = p;

    /* tr/\x01-\x1f\x7f/ /; # Remove control char. */
    while(*p != '\0') {
        if ((*p >= 0x01 && *p <= 0x1f) || *p == 0x7f) {
            *p = ' ';
        }
        p++;
    }

    query_external = nmz_codeconv_external(pos);
    free(alloc);
    if (query_external == NULL) {
        nmz_set_dyingmsg(nmz_msg("%s", strerror(errno)));
        return NULL;
    }

    return query_external;
}

/*
 * Delete string (case insensitive)
 */
static void 
delete_str(char *s, char *d)
{
    int l;
    char *tmp;

    l = strlen(d);
    for (tmp = s; *tmp; tmp++) {
        if (strncasecmp(tmp, d, l) == 0) {
            strcpy(tmp, tmp + l);
            tmp--;
        }
    }
    nmz_chomp(s);
}

static void 
get_value(const char *s, char *value)
{
    int limit = BUFSIZE;
    *value = '\0';
    for (; *s && (limit > 0); s++, limit --) {
        if (nmz_strprefixcasecmp(s, "value=\"") == 0) {
            for (s += strlen("value=\""); *s && *s != '"'; s++, value++) 
	    {
                *value = *s;
            }
            *value = '\0';
            return;
        }
    }
}

static void 
get_select_name(const char *s, char *value)
{
    for (; *s; s++) {
        if (cmp_element(s, (char *)"select name=\"") == 0) {
            s = (char *)strchr(s, '"') + 1;
            for (; *s && *s != (char)'"'; s++, value++) {
                *value = *s;
            }
            *value = '\0';
            return;
        } else if (cmp_element(s, (char *)"/select") == 0) {
            *value = '\0';
            return;
        }
    }
}

static enum nmz_stat
select_option(char *s, const char *name, const char *query, const char *subquery)
{
    char value[BUFSIZE] = "";

    if (cmp_element(s, (char *)"option") == 0) {
	{
            delete_str(s, (char *)" selected=\"selected\"");
            delete_str(s, (char *)" selected='selected'");
            delete_str(s, (char *)" selected");
	}
        fputs(s, stdout);
        get_value(s, value);
        if (strcasecmp(name, "result") == 0) {
            if (strcasecmp(value, get_templatesuffix()) == 0) {
                if (get_htmlmode() == 1) {
                    fputs(" selected", stdout);              /* HTML */
                } else {
                    fputs(" selected=\"selected\"", stdout); /* XHTML */
                }
            }
        } else if (strcasecmp(name, "sort") == 0) {
            if ((strcasecmp(value, "date:late") == 0) && 
		nmz_get_sortmethod() == SORT_BY_DATE &&
		nmz_get_sortorder()  == DESCENDING) 
	    {
                if (get_htmlmode() == 1) {
                    fputs(" selected", stdout);              /* HTML */
                } else {
                    fputs(" selected=\"selected\"", stdout); /* XHTML */
                }
            } else if ((strcasecmp(value, "date:early") == 0) && 
		       nmz_get_sortmethod() == SORT_BY_DATE &&
		       nmz_get_sortorder()  == ASCENDING)
            {
                if (get_htmlmode() == 1) {
                    fputs(" selected", stdout);              /* HTML */
                } else {
                    fputs(" selected=\"selected\"", stdout); /* XHTML */
                }
            } else if ((strcasecmp(value, "score") == 0) && 
		       nmz_get_sortmethod() == SORT_BY_SCORE) 
	    {
                if (get_htmlmode() == 1) {
                    fputs(" selected", stdout);              /* HTML */
                } else {
                    fputs(" selected=\"selected\"", stdout); /* XHTML */
                }
            } else if ((nmz_strprefixcasecmp(value, "field:") == 0) && 
		       nmz_get_sortmethod() == SORT_BY_FIELD) 
	    {
		char *p;
		int n, order = DESCENDING;
		char field[BUFSIZE] = "";

		p = value + strlen("field:");
		n = strspn(p, FIELD_SAFE_CHARS);
		if (n >= BUFSIZE) n = BUFSIZE - 1;
		strncpy(field, p, n);
		field[n] = '\0'; /* Hey, don't forget this after strncpy()! */
		p += n;

		if (nmz_strprefixcasecmp(p, ":ascending") == 0) {
		    order = ASCENDING;
		} else if (nmz_strprefixcasecmp(p, ":descending") == 0) {
		    order = DESCENDING;
		}

		if (strcmp(field, nmz_get_sortfield()) == 0 && 
		    nmz_get_sortorder() == order)
		{
                    if (get_htmlmode() == 1) {
                        fputs(" selected", stdout);              /* HTML */
                    } else {
                        fputs(" selected=\"selected\"", stdout); /* XHTML */
                    }
		}
            }

        } else if (strcasecmp(name, "lang") == 0) {
            if (strcasecmp(value, nmz_get_lang()) == 0) {
                if (get_htmlmode() == 1) {
                    fputs(" selected", stdout);              /* HTML */
                } else {
                    fputs(" selected=\"selected\"", stdout); /* XHTML */
                }
            }
        } else if (strcasecmp(name, "idxname") == 0) {
            if (nmz_get_idxnum() >= 1 && nmz_strsuffixcmp(value, nmz_get_idxname(0)) == 0) {
                if (get_htmlmode() == 1) {
                    fputs(" selected", stdout);              /* HTML */
                } else {
                    fputs(" selected=\"selected\"", stdout); /* XHTML */
                }
            }
        } else if (strcasecmp(name, "subquery") == 0) {
            if (strcasecmp(value, subquery)  == 0) {
                if (get_htmlmode() == 1) {
                    fputs(" selected", stdout);              /* HTML */
                } else {
                    fputs(" selected=\"selected\"", stdout); /* XHTML */
                }
            }
        } else if (strcasecmp(name, "max") == 0) {
            if (atoi(value) == get_maxresult()) {
                if (get_htmlmode() == 1) {
                    fputs(" selected", stdout);              /* HTML */
                } else {
                    fputs(" selected=\"selected\"", stdout); /* XHTML */
                }
            }
        }
        return SUCCESS;
    }
    return FAILURE;
}

/* 
 * Mark CHECKBOX of idxname with CHECKED 
 */
static enum nmz_stat
check_checkbox(char *str, const char *query)
{
    char value[BUFSIZE] = "";
    int i;

    if (cmp_element(str, "input type=\"checkbox\" name=\"idxname\"") == 0) {
        char *pp;
        int db_count, searched;

        if (!(nmz_get_idxnum() == 1 && nmz_get_defaultidx() &&
        strcmp(nmz_get_idxname(0), nmz_get_defaultidx()) == 0)) {
            delete_str(str, (char *)" checked=\"checked\"");
            delete_str(str, (char *)" checked='checked'");
            delete_str(str, (char *)" checked");
	}
        fputs(str, stdout);
        get_value(str, value);
        for (pp = value, db_count = searched = 0 ; *pp ;db_count++) {
            char name[BUFSIZE] = "", *x;
            if ((x = (char *)strchr(pp, (int)','))) {
                *x = '\0';
                strncpy(name, pp, BUFSIZE - 1);
                pp = x + 1;
            } else {
                strncpy(name, pp, BUFSIZE - 1);
                pp += strlen(pp);
            }
            for (i = 0; i < nmz_get_idxnum(); i++) {
                if (strcmp(name, 
			   nmz_get_idxname(i) + 
			   strlen(nmz_get_defaultidx()) + 1) == 0) 
		{
                    searched++;
                    break;
                }
            }
        }
        if (db_count == searched) {
            if (get_htmlmode() == 1) {
                printf(" checked");                /* HTML */
            } else {
                printf(" checked=\"checked\"");    /* XHTML */
            }
        }
        return SUCCESS;
    }
    return FAILURE;
}

/*
 * Handle an HTML tag
 */
static void 
handle_tag(const char *start, const char *end, const char *query, 
               char *select_name, const char *subquery)
{
    char tmp[BUFSIZE] = "";
    int l;

    l = end - start + 1;
    if (l < BUFSIZE - 1) {
        strncpy(tmp, start, l);
        tmp[l] = '\0';
        if (replace_query_value(tmp, query) == SUCCESS)
            return;
        if (select_option(tmp, select_name, query, subquery) == SUCCESS)
            return;
        if (check_checkbox(tmp, query) == SUCCESS)
            return;
        get_select_name(tmp, select_name);
	fputs(tmp, stdout);
    } else {
	fprintf(stdout, "%*.*s", l, l, start);
    }
}

static char *
read_headfoot(const char *fname) 
{
    char *buf, *p, tmpfname[BUFSIZE] = "", suffix[BUFSIZE] = "";
    char *script_name;
    char *document_name;

    if (nmz_choose_msgfile_suffix(fname, suffix) != SUCCESS) {
	nmz_warn_printf("%s: %s", fname, strerror(errno));
	return NULL;
    } 
    strncpy(tmpfname, fname, BUFSIZE - 1);
    strncat(tmpfname, suffix, BUFSIZE - strlen(tmpfname) - 1);

    buf = nmz_readfile(tmpfname); /* buf is allocated in nmz_readfile. */
    if (buf == NULL) { /* failed */
	return NULL;
    }

    /* In case of suffix isn't equal to lang, we needs code conversion */
    if (strcmp(suffix, nmz_get_lang()) != 0) {
	char *new = nmz_codeconv_external(buf); /* new is allocated in nmz_codeconv_external. */
	free(buf);  /* Then we should free buf's memory */
	buf = new;
    }
    
    script_name= getenv("SCRIPT_NAME");
    document_name= getenv("DOCUMENT_URI");

    /* Can't determine script_name */
    if (script_name == NULL) {
	return buf;
    }

    /* no document_name, set it to script_name */
    if (document_name == NULL) {
	document_name = script_name;
    } else {
	/* Delete characters after '?' for Lotus Domino Server R5.06a */
	char *s;
	for (s = document_name; *s; s++)
	    if (*s == '?') {*s = 0; break;}
    }

    /* Expand buf memory for replacing {cgi} and {doc} */
    buf = (char *)realloc(buf, strlen(buf) + strlen(script_name) + strlen(document_name) + 2);
    if (buf == NULL) {
        return NULL;
    }

    /* Replace {cgi} with a proper namazu.cgi location */
    while ((p = strstr(buf, "{cgi}")) != NULL) {
	subst(p, "{cgi}", script_name);
    }

    /* Replace {doc} with the name of the calling document eg, using SSI */
    while ((p = strstr(buf, "{doc}")) != NULL) {
	subst(p, "{doc}", document_name);
    }

    return buf;
}

/* 
 * Substitute pat with rep in str at without memory size consideration.
 */
static void 
subst(char *str, const char *pat, const char *rep)
{
    int patlen, replen;
    patlen = strlen(pat);
    replen = strlen(rep);

    if (patlen == replen) {
	memmove(str, rep, replen);
    } else if (patlen < replen) {
	/* + 1 for including '\0' */
	memmove(str + replen, str + patlen, strlen(str) - patlen + 1);
	memmove(str, rep, replen);
    } else if (patlen > replen) {
	memmove(str, rep, replen);
	/* + 1 for including '\0' */
	memmove(str + replen, str + patlen, strlen(str) - patlen + 1);
    }
}

/*
 *
 * Public functions
 *
 */

/* 
 * Display header or footer file. FIXME: very ad hoc.
 */
void 
print_headfoot(const char * fname, const char * query, const char *subquery)
{
    char *buf, *p, *q, name[BUFSIZE] = "";
    int f, f2;

    buf = read_headfoot(fname);

    if (buf == NULL) {
	return;
    }

    for (p = buf, f = f2 = 0; *p; p++) {
        if (f == 0 && *p == '<') {
            if (nmz_strprefixcasecmp(p, "</title>") == 0) {
		if (*query != '\0') {
		    char *converted = nmz_query_external(query);
                    if (converted) {
                        printf(": &lt;");
                        html_print(converted);
                        printf("&gt;");
                        free(converted);
                    }
		}
		printf("</title>\n");
                p = (char *)strchr(p, '>');
                continue;
            }

            if (!is_formprint() && 
		(nmz_strprefixcasecmp(p, "<form ") == 0)) 
	    {
		f2 = 1;
	    }

            if (!is_formprint() && 
		(nmz_strprefixcasecmp(p, "</form>") == 0)) 
            {
		f2 = 0; 
		p += 6; continue;
	    }
            if (f2) {
		continue;
	    }

            /* 
	     * In case of file's encoding is ISO-2022-JP, 
	     * the problem occurs if JIS X 208 characters in element 
	     */
            q = (char *)strchr(p, (int)'>');
	    fputs("<", stdout);
	    if (*(q-1) != '/') {
		handle_tag(p + 1, q - 1, query, name, subquery);
		fputs(">", stdout);
	    } else {		/* for XHTML */
		handle_tag(p + 1, q - 2, query, name, subquery);
		fputs("/>", stdout);
                set_htmlmode(2);    /* 2: XHTML */
	    }
            p = q;
        } else {
            if ((strncmp(p, "\033$", 2) == 0)
                && (*(p + 2) == 'B' || *(p + 2) == '@')) 
            {
                f = 1;
            } else if ((strncmp(p, "\033(", 2) == 0) &&
                       (*(p + 2) == 'J' || *(p + 2) == 'B' || *(p + 2) == 'H'))
            {
                f = 0;
            }
            if (f2) continue;
            fputc(*p, stdout);
        }
    }
    free(buf);
}
