/*
 * 
 * $Id: rcfile.c,v 1.24 2000-01-10 12:37:24 satoru Exp $
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

#include <assert.h>
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
static void set_pathname(char *dest, const char *command, const char *name);
static FILE *open_rcfile ( const char *argv0 );
static int get_rc_arg ( const char *line, char *arg );
static void replace_home ( char *str );
static struct nmz_strlist *get_rc_args ( const char *line );
static enum nmz_stat parse_rcfile ( const char *line, int lineno );
static enum nmz_stat apply_rc ( int lineno, const char *directive, struct nmz_strlist *args );

static enum nmz_stat process_rc_blank ( const char *directive, const struct nmz_strlist *args );

static enum nmz_stat process_rc_comment(const char *directive, const struct nmz_strlist *args);
static enum nmz_stat process_rc_debug(const char *directive, const struct nmz_strlist *args);
static enum nmz_stat process_rc_index(const char *directive, const struct nmz_strlist *args);
static enum nmz_stat process_rc_base(const char *directive, const struct nmz_strlist *args);
static enum nmz_stat process_rc_alias(const char *directive, const struct nmz_strlist *args);
static enum nmz_stat process_rc_replace(const char *directive, const struct nmz_strlist *args);
static enum nmz_stat process_rc_logging(const char *directive, const struct nmz_strlist *args);
static enum nmz_stat process_rc_scoring(const char *directive, const struct nmz_strlist *args);
static enum nmz_stat process_rc_lang(const char *directive, const struct nmz_strlist *args);

struct conf_directive directive_tab[] = {
    { "BLANK",   0, 0, process_rc_blank },
    { "COMMENT", 0, 0, process_rc_comment },
    { "DEBUG",   1, 0, process_rc_debug },
    { "INDEX",   1, 0, process_rc_index },
    { "BASE",    1, 0, process_rc_base },
    { "ALIAS",   2, 1, process_rc_alias },
    { "REPLACE", 2, 0, process_rc_replace },
    { "LOGGING", 1, 0, process_rc_logging },
    { "SCORING", 1, 0, process_rc_scoring },
    { "LANG",    1, 0, process_rc_lang },
    { NULL,      0, 0, NULL }
};



static enum nmz_stat
process_rc_blank(const char *directive, const struct nmz_strlist *args)
{
    /* Do nothing */
    return SUCCESS;
}

static enum nmz_stat
process_rc_comment(const char *directive, const struct nmz_strlist *args)
{
    /* Do nothing */
    return SUCCESS;
}

static enum nmz_stat
process_rc_debug(const char *directive, const struct nmz_strlist *args)
{
    char *arg1 = args->value;

    if (strcasecmp(arg1, "ON") == 0) {
	nmz_set_debugmode(1);
    } else if (strcasecmp(arg1, "OFF") == 0) {
	nmz_set_debugmode(0);
    }
    return SUCCESS;
}

static enum nmz_stat
process_rc_index(const char *directive, const struct nmz_strlist *args)
{
    char *arg1 = args->value;

    strcpy(DEFAULT_INDEX, arg1);
    return SUCCESS;
}

static enum nmz_stat
process_rc_base(const char *directive, const struct nmz_strlist *args)
{
    char *arg1 = args->value;

    strcpy(BASE_URI, arg1);
    return SUCCESS;
}

/*
 * FIXME: one-to-multiple alias should be allowed.
 */
static enum nmz_stat
process_rc_alias(const char *directive, const struct nmz_strlist *args)
{
    char *arg1, *arg2;

    arg1 = args->value;
    arg2 = args->next->value;
    if (nmz_add_alias(arg1, arg2) != SUCCESS) {
	return FAILURE;
    }
    return SUCCESS;
}

static enum nmz_stat
process_rc_replace(const char *directive, const struct nmz_strlist *args)
{
    char *arg1, *arg2;

    arg1 = args->value;
    arg2 = args->next->value;
    if (nmz_add_replace(arg1, arg2) != SUCCESS) {
	return FAILURE;
    }
    return SUCCESS;
}

static enum nmz_stat
process_rc_logging(const char *directive, const struct nmz_strlist *args)
{
    char *arg1 = args->value;

    if (strcasecmp(arg1, "ON") == 0) {
	nmz_set_loggingmode(1);
    } else if (strcasecmp(arg1, "OFF") == 0) {
	nmz_set_loggingmode(0);
    }

    return SUCCESS;
}

static enum nmz_stat
process_rc_scoring(const char *directive, const struct nmz_strlist *args)
{
    char *arg1 = args->value;

    if (strcasecmp(arg1, "TFIDF") == 0) {
	TfIdf = 1;
    } else if (strcasecmp(arg1, "SIMPLE") == 0) {
	TfIdf = 0;
    }
    return SUCCESS;
}

static enum nmz_stat
process_rc_lang(const char *directive, const struct nmz_strlist *args)
{
    char *arg1 = args->value;

    nmz_set_lang(arg1);
    return SUCCESS;
}

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
 * Make the pathname `dest' by conjuncting `command' and `name'.
 */
static void 
set_pathname(char *dest, const char *command, const char *name)
{
    int i;

    strcpy(dest, command);
    for (i = strlen(dest) - 1; i > 0; i--) {
	if (dest[i] == '/') {
	    i++;
	    break;
	}
    }
    strcpy(dest + i, name);
    return;
}

/*
 * 1. current_executing_binary_dir/.namazurc
 * 2. ${HOME}/.namazurc
 * 3. DEFAULT_NAMAZURC (SYSCONFDIR/namazurc)
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


/*
 * Get the directive and the following args and return them as a list.
 *
 * NOTE: the string `line' should be chomped before calling the function.
 */
static struct nmz_strlist*
get_rc_args(const char *line)
{
    struct nmz_strlist *list = NULL;
    int n;

    /* Skip white spaces in the beginning of this line */
    n = strspn(line, " \t");    /* skip white spaces */
    line += n;
    /* Determine whether or not this line is only a blank */
    if (*line == '\0') {
	return nmz_push_strlist(list, "BLANK");
    }

    /* Determine whether or not this line is only a comment */
    if (*line == '#') {
	return nmz_push_strlist(list, "COMMENT");
    }

    /* Get a directive name. */
    {
	char directive[BUFSIZE];
	n = strspn(line, DIRECTIVE_CHARS);
	if (n == 0) {
	    errmsg = "invalid directive name";
	    return 0;
	}
	strncpy(directive, line, n);
	directive[n] = '\0';  /* Hey, don't forget this after strncpy()! */
	list = nmz_push_strlist(list, directive);
	line += n;
    }

    /* Skip a delimiter after a directive */
    n = strspn(line, " \t");    /* skip white spaces */
    line += n;
    if (n == 0) {
	errmsg = "can't find a delimiter after directive";
	return 0;
    }

    while (1) {
	char arg[BUFSIZE];
	n = get_rc_arg(line, arg);
	if (n == 0) { /* cannot get arg1 */
	    return NULL;
	}
	line += n;

	/* Replace ~/ */
	replace_home(arg);

	list = nmz_push_strlist(list, arg);

	/* Skip a delimiter after the arg */
	n = strspn(line, " \t");    /* skip white spaces */
	line += n;

	if (*line == '\0' || *line == '#') {  /* allow comment at the ending */
	    /* This line has only one argument (arg) */
	    break;
	}
    }

    return list;
}

static enum nmz_stat 
parse_rcfile(const char *line, int lineno) 
{
    struct nmz_strlist *args;
    char *directive;

    args = get_rc_args(line);
    if (args == NULL) { 
	return FAILURE; /* error */
    }
    directive = args->value; /* the first arg is a directive. */
    args = args->next;

    if (apply_rc(lineno, directive, args) != SUCCESS) {
        return FAILURE;
    }

    /* 
     * NOTE: Cannot turn is_debugmode() on with -d option because
     * rcfile is loaded before command line options are parsed.
     * But we can set debugmode by setting "Debug on" in the TOP of 
     * namazurc.
     */
    if (nmz_is_debugmode() &&
	(strcasecmp(directive, "COMMENT") != 0) &&
	(strcasecmp(directive, "BLANK") != 0))
    {
	struct nmz_strlist *ptr;
	int i;

	printf("%4d: Directive:  [%s]\n", lineno, directive);

	for (ptr = args, i = 1 ; ptr != NULL; i++) {
	    printf("      Argument %d: [%s]\n", i, ptr->value);
	    ptr = ptr->next;
	}
    }

    return SUCCESS;
}

static enum nmz_stat 
apply_rc(int lineno, const char *directive, struct nmz_strlist *args)
{
    int argnum = 0;
    struct nmz_strlist *ptr;
    struct conf_directive *dtab = directive_tab;

    for (ptr = args; ptr != NULL; ptr = ptr->next) {
	argnum++;
    }

    for (; dtab->name != NULL;  dtab++) {
	if (strcasecmp(dtab->name, directive) == 0) {
	    /* 
	     * Check whether the number of argument is right.
	     */
	    if (argnum == dtab->argnum ||
		(dtab->plus && argnum > dtab->argnum)) 
	    {
		/* If number of argument is right, apply appropriate func. */
		if ((*(dtab->func))(directive, args) != SUCCESS) {
		    return FAILURE;
		}
		return SUCCESS;
	    } else if (argnum < dtab->argnum) {
		errmsg = "too few arguments";
		return FAILURE;
	    } else if (argnum > dtab->argnum) {
		errmsg = "too many arguments";
		return FAILURE;
	    } else {
		assert(0);
		/* NOTREACHED */
		return FAILURE;
	    }
	}
    }

    errmsg = "unknown directive";
    return FAILURE;
}

/*
 *
 * Public functions
 *
 */

char *
nmz_set_namazurc(const char *arg)
{
    return strcpy(namazurc, arg);
}

/* 
 * Load rcfile of namazu 
 * FIXME: Taking the argv0 parameter is dirty spec.
 */
enum nmz_stat 
nmz_load_rcfile(const char *argv0)
{
    FILE *fp;
    char buf[BUFSIZE];
    int lineno = 1;

    fp = open_rcfile(argv0);
    if (fp == NULL)
	return SUCCESS; /* no rcfile exists */

    rcfile_is_loaded = 1;  /* for nmz_show_rcfile() */

    while (fgets(buf, BUFSIZE, fp) != NULL) {
	int current_lineno = lineno;
	
	do {
	    lineno++;
	    nmz_chomp(buf);
	    if (buf[strlen(buf) - 1] == '\\') { /* ending with \ */
		int remaining;
		
		buf[strlen(buf) - 1] = '\0'; /* Remove ending \ */
		remaining = BUFSIZE - strlen(buf);
		if (fgets(buf + strlen(buf), remaining, fp) == NULL) {
		    nmz_chomp(buf);
		    break;
		}
	    } else {
		break;
	    }
	} while (1);

	nmz_conv_ja_any_to_eucjp(buf);  /* for Shift_JIS encoding */
	if (parse_rcfile(buf, current_lineno) != SUCCESS) {
	    nmz_set_dyingmsg(nmz_msg("%s:%d: syntax error: %s.",  
				     namazurc, current_lineno, errmsg));
	    return FAILURE;
	}
    }
    fclose(fp);
    return SUCCESS;
}

void 
nmz_show_rcfile(void)
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
"), DEFAULT_INDEX, BASE_URI, nmz_is_loggingmode() ? "on" : "off",
           nmz_get_lang(), TfIdf ? "tfidf" : "simple");

    nmz_show_aliases();
    nmz_show_replaces();

    /*    exit(0);*/
    return;
}

