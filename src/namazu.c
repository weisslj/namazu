/*
 * 
 * namazu.c - search client of Namazu
 *
 * $Id: namazu.c,v 1.29 1999-10-11 04:25:26 satoru Exp $
 * 
 * Copyright (C) 1997-1999 Satoru Takabayashi  All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>

#include "namazu.h"
#include "getopt.h"
#include "util.h"
#include "codeconv.h"
#include "form.h"
#include "usage.h"
#include "conf.h"
#include "output.h"
#include "search.h"
#include "cgi.h"
#include "hlist.h"
#include "i18n.h"

/* redirect stdio to specified file */
int set_redirect_stdout_to_file(uchar * fname)
{
    int fd;

    if (-1 == (fd = open(fname, O_CREAT | O_TRUNC | O_WRONLY, 00600))) {
	diemsg("stdio2file(cannot open)");
	return 1;
    }
    close(STDOUT);
    dup(fd);
    close(STDERR);
    dup(fd);
    close(fd);
    return 0;
}

/*
 * Command line options.
 */
static const char *short_options = "01:2345acCedf:FhHlL:n:o:qrRsSUvw:";
static struct option long_options[] = {
    {"help",             no_argument,       NULL, '0'},
    {"result",           required_argument, NULL, '1'},
    {"late",             no_argument,       NULL, '2'},
    {"early",            no_argument,       NULL, '3'},
    {"sort",             required_argument, NULL, '4'},
    {"ascending",        no_argument,       NULL, '5'},
    {"all",              no_argument,       NULL, 'a'},
    {"count",            no_argument,       NULL, 'c'},
    {"show-config",      no_argument,       NULL, 'C'},
    {"debug",            no_argument,       NULL, 'd'},
    {"config",           required_argument, NULL, 'f'},
    {"form",             no_argument,       NULL, 'F'},
    {"html",             no_argument,       NULL, 'h'},
    {"page",             no_argument,       NULL, 'H'},
    {"list",             no_argument,       NULL, 'l'},
    {"lang",             required_argument, NULL, 'L'},
    {"max",              required_argument, NULL, 'n'},
    {"output",           required_argument, NULL, 'o'},
    {"quiet",            no_argument,       NULL, 'q'},
    {"no-references",    no_argument,       NULL, 'r'},
    {"no-replace",       no_argument,       NULL, 'R'},
    {"short",            no_argument,       NULL, 's'},
    {"no-encode-uri",    no_argument,       NULL, 'U'},
    {"version",          no_argument,       NULL, 'v'},
    {"whence",           required_argument, NULL, 'w'},
    {NULL, 0, NULL, 0}
};

/* parse command line options */
int parse_options(int argc, char **argv)
{
    for (;;) {
        int ch = getopt_long(argc, argv, short_options, long_options, NULL);
        if (ch == EOF) {
            break;
	}
	switch (ch) {
	case '0':
	    show_long_usage();
	    return DIE_NOERROR;
	    break;
	case '1':
	    strcpy(Template, optarg);
	    break;
	case '2':
	    SortMethod    = SORT_BY_DATE;
	    SortOrder = DESCENDING;
	    break;	  
	case '3':	  
	    SortMethod    = SORT_BY_DATE;
	    SortOrder = ASCENDING;
	    break;
	case '4':  /* --sort */
	{
	    if (strcasecmp(optarg, "score") == 0) {
		SortMethod = SORT_BY_SCORE;
	    } else if (strcasecmp(optarg, "date") == 0) {
		SortMethod    = SORT_BY_DATE;
	    } else if (strprefixcasecmp(optarg, "field:") == 0) {
		SortMethod    = SORT_BY_FIELD;
		set_sort_field(optarg + 6);
	    }
	}
	break;
	case '5':  /* --ascending */
	    SortOrder = ASCENDING;
	    break;
	case 'f':
	    strcpy(NAMAZURC, optarg);
	    break;
	case 'n':
	    HListMax = atoi(optarg);
	    break;
	case 'w':
	    HListWhence = atoi(optarg);
	    break;
	case 'd':
	    Debug = 1;
	    break;
	case 's':
	    strcpy(Template, "short");
	    break;
	case 'l':
	case 'S':  /* 'S' for backward compatibility */
	    ListFormat = 1;
	    break;
	case 'q':
	    Quiet = 1;
	    break;
	case 'c':
	    HitCountOnly = 1;
	    break;
	case 'h':
	    HtmlOutput = 1;
	    break;
	case 'H':
	    HidePageIndex = 0;
	    break;
	case 'F':
	    ForcePrintForm = 1;
	    break;
	case 'a':
	    AllList = 1;
	    break;
	case 'R':
	    NoReplace = 1;
	    break;
	case 'r':
	    NoReference = 1;
	    break;
	case 'U':
	    DecodeURI = 0;
	    break;
	case 'v':
	    printf("namazu v%s\n", VERSION);
	    return DIE_NOERROR;
	    break;
	case 'C':
	    if (load_conf(argv[0]))
	      return DIE_ERROR;
	    show_conf();
	    return DIE_NOERROR;
	    break;
	case 'L':
	    set_lang(optarg);
	    break;
	case 'o':
	    if (set_redirect_stdout_to_file(optarg))
	      return DIE_ERROR;
	    break;
	}
    } 

    return optind;
}

void free_idxnames(void)
{
    int i;
    for (i = 0; i < Idx.num; i++) {
        free(Idx.names[i]);
	free_phraseres(Idx.pr[i]);
    }
    Idx.num = 0;
}

void free_aliases(void)
{
    ALIAS *list, *next;
    list = Alias;

    while (list) {
	next = list->next;
	free(list->alias);
	free(list->real);
	free(list);
	list = next;
    }
}

void free_replaces(void)
{
    REPLACE *list, *next;
    list = Replace;

    while (list) {
	next = list->next;
	free(list->pat);
	free(list->rep);
	if (list->pat_re != NULL) {
	    re_free_pattern(list->pat_re);
	}
	free(list);
	list = next;
    }
}

void make_fullpathname_msg(void)
{
    uchar *base;
    
    if (Idx.num == 1) {
        base = Idx.names[0];
    } else {
        base = DEFAULT_INDEX;
    }
    
    pathcat(base, NMZ.head);
    pathcat(base, NMZ.foot);
    pathcat(base, NMZ.body);
    pathcat(base, NMZ.lock);
    pathcat(base, NMZ.tips);
}

void codeconv_query(uchar *query)
{
    if (is_lang_ja()) {
        if (conv_ja_any_to_eucjp(query)) {
            zen2han(query);
        }
    }
}

/* namazu core routine */
int namazu_core(uchar * query, uchar *subquery, uchar *av0)
{
    uchar query_with_subquery[BUFSIZE * 2];
    HLIST hlist;

    /* make full-pathname of NMZ.{head,foot,msg,body,slog}.?? */
    make_fullpathname_msg();

    codeconv_query(query);
    codeconv_query(subquery);
    strcpy(query_with_subquery, query);
    strcat(query_with_subquery, " ");
    strcat(query_with_subquery, subquery);

    /* if query is null, show NMZ.head,body,foot and exit with error */
    if (*query == '\0') {
        if (HtmlOutput) {
            print(MSG_MIME_HEADER);
            print_headfoot(NMZ.head, query, subquery);
            print_msgfile(NMZ.body);
            print_headfoot(NMZ.foot, query, subquery);
        }
	free_idxnames();
	/*	exit(1);*/
	return 1;
    }

    debug_printf(" -n: %d\n", HListMax);
    debug_printf(" -w: %d\n", HListWhence);
    debug_printf("query: [%s]\n", query);

    /* search */
    hlist = search_main(query_with_subquery);
    if (hlist.n == DIE_HLIST)
        return DIE_ERROR;

    if (HtmlOutput && IsCGI)
	print(MSG_MIME_HEADER);
    if (HtmlOutput)
	print_headfoot(NMZ.head, query, subquery);

    /* result1 */
    if (!HitCountOnly && !ListFormat && !NoReference && !Quiet) {
        print_result1();

        if (Idx.num > 1) {
            print("\n");
            if (HtmlOutput)
                print("<ul>\n");
        }
    }

    print_hit_count();

    /* result2 */
    if (!HitCountOnly && !ListFormat && !NoReference && !Quiet) {
        if (Idx.num > 1 && HtmlOutput) {
            print("</ul>\n");
        }
        print_result2();
    }

    if (hlist.n > 0) {
        if (!HitCountOnly && !ListFormat && !Quiet) {
            print_hitnum(hlist.n);  /* <!-- HIT -->%d<!-- HIT --> */
        }
	if (HitCountOnly) {
	    printf("%d\n", hlist.n);
	} else {
	    print_listing(hlist); /* summary listing */
	}
        if (!HitCountOnly && !ListFormat && !Quiet) {
            print_range(hlist);
        }
    } else {
        if (HitCountOnly) {
            print("0\n");
        } else if (!ListFormat) {
            html_print(_("	<p>No document matching your query.</p>\n"));
	    if (HtmlOutput) {
		print_msgfile(NMZ.tips);
	    }
        }
    }

    free_hlist(hlist);

    if (HtmlOutput) {
	print_headfoot(NMZ.foot, query, subquery);
    }
    free_idxnames();
    free_aliases();
    free_replaces();

    return 0;
}


/* get an environmental variable of NAMAZUCONFPATH
 * original by Shimizu-san [1998-02-27]
 */
void getenv_namazuconf(void)
{
    uchar *env_namazu_conf;

    env_namazu_conf = (uchar*)getenv("NAMAZUCONFPATH");
    if (env_namazu_conf == NULL)
        env_namazu_conf = (uchar*)getenv("NAMAZUCONF");

    if (env_namazu_conf != NULL)
        strcpy(NAMAZURC, env_namazu_conf);
}

void uniq_idxnames(void)
{
    int i, j, k;

    for (i = 0; i < Idx.num - 1; i++) {
        for (j = i + 1; j < Idx.num; j++) {
            if (strcmp(Idx.names[i], Idx.names[j]) == 0) {
                free(Idx.names[j]);
                for (k = j + 1; k < Idx.num; k++) {
                    Idx.names[k - 1] = Idx.names[k];
                }
                Idx.num--;
                j--;
            }
        }
    }
}

int expand_idxname_aliases(void)
{
    int i;

    for (i = 0; i < Idx.num; i++) {
	ALIAS *list = Alias;
	while (list) {
	    if (strcmp(Idx.names[i], list->alias) == 0) {
		free(Idx.names[i]);
		Idx.names[i] = (uchar *) malloc(strlen(list->real) + 1);
		if (Idx.names[i] == NULL) {
		    diemsg("expand_idxname_aliases: malloc()");
		    return DIE_ERROR;
		}
		strcpy(Idx.names[i], list->real);
            }
	    list = list->next;
	}
    }
    return 0;
}

int complete_idxnames(void)
{
    int i;

    for (i = 0; i < Idx.num; i++) {
 	if (*Idx.names[i] == '+' && isalnum(*(Idx.names[i] + 1))) {
	    uchar *tmp;
	    tmp = (uchar *)malloc(strlen(DEFAULT_INDEX) 
				  + 1 + strlen(Idx.names[i]) + 1);
	    if (tmp == NULL) {
		diemsg("complete_idxnames: malloc()");
		return DIE_ERROR;
	    }
	    strcpy(tmp, DEFAULT_INDEX);
	    strcat(tmp, "/");
	    strcat(tmp, Idx.names[i] + 1);  /* +1 means '+' */
	    free(Idx.names[i]);
	    Idx.names[i] = tmp;
	}
    }
    return 0;
}

void suicide ()
{
    diemsg("processing time exceeds a limit: %d", SUICIDE_TIME);
    diewithmsg();
}

int main(int argc, char **argv)
{
    int i = 0;
    int ret;
    uchar query[BUFSIZE] = "", subquery[BUFSIZE] = "";

    set_lang("");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    getenv_namazuconf();

    Idx.num = 0;

    if (argc == 1) {
	if (load_conf(argv[0]))
	    diewithmsg();
	IsCGI = 1;	/* if no argument, assume this session as CGI */
	HtmlOutput = 1;
    } else {
	HtmlOutput = 0;		 /* do not display result by HTML format */
	DecodeURI = 1;		 /* decode a URI */
	HidePageIndex = 1;	 /* do not diplay page index */

	i = parse_options(argc, argv); 
	if (i == DIE_ERROR)
	    diewithmsg();
	if (i == DIE_NOERROR)
	    exit(0);
	if (load_conf(argv[0]))
	    diewithmsg();

	if (i == argc) {
	    show_mini_usage();
	    exit(1);
	}
	if (strlen(argv[i]) > QUERY_MAX) {
	    html_print(_(MSG_TOO_LONG_QUERY));
	    return 1;
	}
        strcpy(query, argv[i++]);
        if (i < argc) {
            for (Idx.num = 0; i < argc && Idx.num < INDEX_MAX; i++) {
		Idx.names[Idx.num] = 
		    (uchar *) malloc(strlen(argv[i]) + 1);
		if (Idx.names[Idx.num] == NULL) {
		    die("main: malloc(idxname)");
		}
		Idx.pr[Idx.num] = NULL;
		strcpy(Idx.names[Idx.num], argv[i]);
		Idx.num++;
            }
        } 
        if (Idx.num == 0) {
            Idx.num = 0;
            Idx.names[Idx.num] = 
                (uchar *) malloc(strlen(DEFAULT_INDEX) + 1);
            if (Idx.names[Idx.num] == NULL) {
                die("main: malloc(idxname)");
            }
            strcpy(Idx.names[Idx.num], DEFAULT_INDEX);
            Idx.num = 1;
	}
    }
    if (IsCGI) {
	init_cgi(query, subquery);
    }

    uniq_idxnames();
    if (expand_idxname_aliases())
        diewithmsg();
    if (complete_idxnames())
        diewithmsg();

    if (Debug) {
        for (i = 0; i < Idx.num; i++) {
            debug_printf("Idx.names[%d]: %s\n", i, Idx.names[i]);
        }
    }

    if (IsCGI) {
	/* set a suicide timer */
	signal(SIGALRM, suicide);
	alarm(SUICIDE_TIME);
    }

    ret = namazu_core(query, subquery, argv[0]);
    if (ret == DIE_ERROR)
        diewithmsg();
    return ret;
}

