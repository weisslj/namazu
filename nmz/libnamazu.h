/*
 * 
 * libnamazu.h - Namazu library api
 *
 * $Id: libnamazu.h,v 1.1 1999-11-01 07:23:55 knok Exp $
 * 
 */

#ifdef _LIBNAMAZU_H
#define _LIBNAMAZU_H

void free_idxnames(void);
void free_aliases(void);
void free_replaces(void);
void make_fullpathname_msg(void);
void codeconv_query(uchar *query);
void getenv_namazuconf(void);
void uniq_idxnames(void);
int expand_idxname_aliases(void);
int complete_idxnames(void);
char *set_namazurc(char *arg)
char *set_template(char *arg)
void set_sortbydate(void);
void set_sortbyscore(void);
void set_sortbyfield(void);
void set_sort_descending(void);
void set_sort_ascending(void);
void set_debug(void);

#endif _LIBNAMAZU_H
