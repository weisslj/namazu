/*
 * 
 * $Id: rcfile.c,v 1.13 2000-01-07 23:59:33 satoru Exp $
 * 
 * Copyright (C) 1997-2000 Satoru Takabayashi  All rights reserved.
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "libnamazu.h"
#include "rcfile.h"
#include "util.h"
#include "regex.h"
#include "re.h"
#include "codeconv.h"
#include "i18n.h"
#include "var.h"
#include "alias.h"
#include "replace.h"

static char *errmsg  = NULL;
static int rcfile_is_loaded = 0;
static char namazurc[BUFSIZE] = "";

/*
 *
 * Private functions
 *
 */

static void getenv_namazurc ( void );
static void set_pathname ( char *to, const char *o, const char *name );
static FILE *open_rcfile ( const char *argv0 );
static int get_rc_arg ( const char *line, char *arg );
static void replace_home ( char *str );
static int get_rc_args ( const char *line, char *directive, char *arg1, char *arg2 );
static enum nmz_stat parse_rcfile ( const char *line, int lineno );
static int is_valid_argnum ( const char *directive, int argnum );
static enum nmz_stat apply_rc ( const char *directive, int lineno, const char *arg1, const char *arg2 );

/* 
 * Get an environmental variable of NAMAZUCONFPATH
 * contributed by Shimizu-san [1998-02-27]
 */
static void 
getenv_namazurc(void)
{
    char *env_namazu_conf;

    env_namazu_conf = getenv("NAMAZUCONFPATH");
    if (env_namazu_conf == NULL)
        env_namazu_conf = getenv("NAMAZUCONF");

    if (env_namazu_conf != NULL)
        strcpy(namazurc, env_namazu_conf);
}

/*
 * Change filename in full pathname
 */
static void 
set_pathname(char *to, const char *orig, const char *name)
{
    int i;

    strcpy(to, orig);
    for (i = strlen(to) - 1; i > 0; i--) {
	if (to[i] == '/') {
	    i++;
	    break;
	}
    }
    strcpy(to + i, name);
    return;
}

/*
 1. current_executing_binary_dir/.namazurc
 2. ${HOME}/.namazurc
 3. DEFAULT_NAMAZURC (SYSCONFDIR/namazurc)
 */
static FILE *
open_rcfile(const char *argv0)
{
    FILE *fp;
    char fname[BUFSIZE], *home;

    getenv_namazurc();

    /* Rcfile is specified (not default) */
    if (namazurc[0] != '\0') {
        if ((fp = fopen(namazurc, "rb"))) {
            return fp;
        }
    }

    /* Check the where program is */
    set_pathname(fname, argv0, ".namazurc");
    if ((fp = fopen(fname, "rb"))) {
        strcpy(namazurc, fname);
        return fp;
    }

    /* Checke a home directory */
    if ((home = getenv("HOME")) != NULL) {
        strcpy(fname, home);
        strcat(fname, "/.namazurc");
        if ((fp = fopen(fname, "rb"))) {
            strcpy(namazurc, fname);
            return fp;
        }
    }

    /* Check the defalut */
    strcpy(fname, CONFDIR);
    strcat(fname, "/namazurc");
    if ((fp = fopen(fname, "rb"))) {
	strcpy(namazurc, fname);
        return fp;
    }
    return (FILE *) NULL;
}

static int 
get_rc_arg(const char *line, char *arg)
{
    *arg = '\0';
    if (*line != '"') {
	int n = strcspn(line, " \t");
	strncpy(arg, line, n);
	arg[n] = '\0';     /* Hey, don't forget this after strncpy()! */
	return n;
    } else {  /* double quoted argument */
	int n;
	line++;
	n = 1;
	do {
	    int nn = strcspn(line, "\"\\");
	    strncat(arg, line, nn);
	    n += nn;
	    line += nn;
	    arg[n] = '\0';
	    if (*line == '\0') {  /* terminator not matched */
		errmsg = "can't find string terminator";
		return 0;
	    }
	    if  (*line == '"') {  /* ending */
		n++;
		return n;
	    }
	    if (*line == '\\') {  /* escaping character */
		strncat(arg, line + 1, 1);
		n += 2;
		line += 2;
	    }
	} while (1);
	return n;
    }
}

static void 
replace_home(char *str)
{
    char tmp[BUFSIZE];

    strcpy(tmp, str);
    if (nmz_strprefixcmp(tmp, "~/") == 0) {
	char *home;
	/* Checke a home directory */
	if ((home = getenv("HOME")) != NULL) {
	    strcpy(tmp, home);
	    strcat(tmp, "/");
	    strcat(tmp, str + strlen("~/"));
	    strcpy(str, tmp);
	    return;
	}
    }

    return;
}


static int 
get_rc_args(const char *line, char *directive, 
	    char *arg1, char *arg2)
{
    int n;

    /* Skip white spaces in the beginning of this line */
    n = strspn(line, " \t");    /* skip white spaces */
    line += n;
    /* Determine whether or not this line is only a blank */
    if (*line == '\0') {
	strcpy(directive, "BLANK");
	return 0;
    }

    /* Determine whether or not this line is only a comment */
    if (*line == '#') {
	strcpy(directive, "COMMENT");
	return 0;
    }

    /* Get a directive name */
    n = strspn(line, DIRECTIVE_CHARS);
    if (n == 0) {
	errmsg = "invalid directive name";
	return 0;
    }
    strncpy(directive, line, n);
    directive[n] = '\0';        /* Hey, don't forget this after strncpy()! */
    line += n;

    /* Skip a delimiter after a directive */
    n = strspn(line, " \t");    /* skip white spaces */
    line += n;
    if (n == 0) {
	errmsg = "can't find a delimiter after directive";
	return 0;
    }

    /* Get arg1 */
    n = get_rc_arg(line, arg1);
    line += n;

    if (n == 0) { /* cannot get arg1 */
	return 0;
    }

    /* Replace ~/ */
    replace_home(arg1);

    /* Skip a delimiter after an arg1 */
    n = strspn(line, " \t");    /* skip white spaces */
    line += n;

    if (*line == '\0' || *line == '#') {  /* allow comment at the ending */
        /* This line has only one argument (arg1) */
	return 1;
    }

    /* Get arg2 */
    n = get_rc_arg(line, arg2);
    line += n;

    if (n == 0) { /* cannot get arg2 */
	return 1;
    }

    /* Replace ~/ */
    replace_home(arg2);

    /* Skip trailing white spaces after an arg2 */
    n = strspn(line, " \t");    /* skip white spaces */
    line += n;

    if (*line != '\0' || *line != '#') {  /* allow comment at the ending */
	return 2;
    } else {
	errmsg = "trailing garbages exist";
	return 0;
    }
}

static enum nmz_stat 
parse_rcfile(const char *line, int lineno) 
{
    char directive[BUFSIZE] = "";
    char arg1[BUFSIZE] = "";
    char arg2[BUFSIZE] = "";
    int argnum = 0;

    argnum = get_rc_args(line, directive, arg1, arg2);
    if (errmsg != NULL) { 
	return FAILURE; /* error */
    }

    if (is_debugmode() && 
	(strcasecmp(directive, "COMMENT") != 0) &&
	(strcasecmp(directive, "BLANK") != 0))
    {
	printf("%d: DIRECTIVE: [%s]\n", lineno, directive);
	if (argnum >= 1) {
	    printf("    ARG1: [%s]\n", arg1);
	}
	if (argnum == 2) {
	    printf("    ARG2: [%s]\n", arg2);
	}
	nmz_print("\n");
    }

    if (!is_valid_argnum(directive, argnum)) {
	return FAILURE; /* error */
    }

    if (apply_rc(directive, lineno, arg1, arg2) != SUCCESS)
        return FAILURE;
    return SUCCESS;
}

static int 
is_valid_argnum(const char *directive, int argnum)
{
    struct conf_directive {
	char *name;
	int argnum;
    };
    struct conf_directive dtab[] = {
	{ "BLANK",   0 },
	{ "COMMENT", 0 },
	{ "INDEX",   1 },
	{ "BASE",    1 },
	{ "ALIAS",   2 },
	{ "REPLACE", 2 },
	{ "LOGGING", 1 },
	{ "SCORING", 1 },
	{ "LANG",    1 },
	{ NULL,      0 }
    };
    int i;

    for (i = 0; dtab[i].name != NULL; i++) {
	if (strcasecmp(dtab[i].name, directive) == 0) {
	    if (argnum == dtab[i].argnum) {
		return 1;  /* OK */
	    } else if (argnum < dtab[i].argnum) {
		errmsg = "too few arguments";
		return 0;  /* NG */
	    } else if (argnum > dtab[i].argnum) {
		errmsg = "too many arguments";
		return 0;  /* NG */
	    } else {
		assert(0);
		/* NOTREACHED */
		return 0;
	    }
	}
    }
    assert(0);
    /* NOTREACHED */
    return 0;
}

/*
 * FIXME: This processing should be rewritten with function pointers.
 */
static enum nmz_stat 
apply_rc(const char *directive, int lineno, 
	 const char *arg1, const char *arg2)
{
    if (strcasecmp(directive, "COMMENT") == 0) {
	;  /* only a comment */
    } else if (strcasecmp(directive, "INDEX") == 0) {
	strcpy(DEFAULT_INDEX, arg1);
    } else if (strcasecmp(directive, "BASE") == 0) {
	strcpy(BASE_URI, arg1);
    } else if (strcasecmp(directive, "REPLACE") == 0) {
	if (add_replace(arg1, arg2) != SUCCESS)
	    return FAILURE;
    } else if (strcasecmp(directive, "ALIAS") == 0) {
	if (add_alias(arg1, arg2) != SUCCESS)
	    return FAILURE;
    } else if (strcasecmp(directive, "LOGGING") == 0) {
	if (strcasecmp(arg1, "ON") == 0) {
	    set_loggingmode(1);
	} else if (strcasecmp(arg1, "OFF") == 0) {
	    set_loggingmode(0);
	}
    } else if (strcasecmp(directive, "SCORING") == 0) {
	if (strcasecmp(arg1, "TFIDF") == 0) {
	    TfIdf = 1;
	} else if (strcasecmp(arg1, "SIMPLE") == 0) {
	    TfIdf = 0;
	}
    } else if (strcasecmp(directive, "LANG") == 0) {
	set_lang(arg1);
    }
    return 0;
}

/*
 *
 * Public functions
 *
 */

char *
set_namazurc(const char *arg)
{
    return strcpy(namazurc, arg);
}

/* 
 * load rcfile of namazu 
 * FIXME: Taking argv0 argument is dirty spec.
 */
enum nmz_stat 
load_rcfile(const char *argv0)
{
    FILE *fp;
    char buf[BUFSIZE];
    int lineno = 0;

    fp = open_rcfile(argv0);
    if (fp == NULL)
	return SUCCESS; /* no rcfile exists */

    rcfile_is_loaded = 1;  /* for show_rcfile() */

    while (fgets(buf, BUFSIZE, fp)) {
	lineno++;
	nmz_chomp(buf);
	conv_ja_any_to_eucjp(buf);  /* for Shift_JIS encoding */
	if (parse_rcfile(buf, lineno) != SUCCESS)
	    return FAILURE;
	if (errmsg != NULL) { /* error occurred */
	    set_dyingmsg("%s:%d: syntax error: %s.\n",  
		   namazurc, lineno, errmsg);
	    return FAILURE;
	}
    }
    fclose(fp);
    return SUCCESS;
}

void 
show_rcfile(void)
{
    if (rcfile_is_loaded == 1) {
	printf(_("Config:  %s\n"), namazurc);
    }

    printf(_("\
Default: %s\n\
BASE:    %s\n\
Logging: %s\n\
Lang:    %s\n\
Scoring: %s\n\
"), DEFAULT_INDEX, BASE_URI, is_loggingmode() ? "on" : "off",
           get_lang(), TfIdf ? "tfidf" : "simple");

    show_aliases();
    show_replaces();

    /*    exit(0);*/
    return;
}

