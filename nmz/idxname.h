/*
 * 
 * idxname.h - header files for Idx handling.
 *
 * $Id: idxname.h,v 1.6 2000-01-06 03:44:29 satoru Exp $
 * 
 * 
 */

#ifndef _IDXNAME_H
#define _IDXNAME_H

extern enum nmz_stat add_index(const char *idxname);
extern int get_idxnum();
extern void free_idxnames ( void );
extern void uniq_idxnames ( void );
extern int expand_idxname_aliases ( void );
extern int complete_idxnames ( void );
extern char *get_idxname(int num);
extern int get_idx_totalhitnum(int id);
extern void set_idx_totalhitnum(int id, int hitnum);
extern struct nmz_hitnumlist *get_idx_hitnumlist(int id);
extern void set_idx_hitnumlist(int id, struct nmz_hitnumlist *hnlist);
extern struct nmz_hitnumlist *push_hitnum ( struct nmz_hitnumlist *hn, const char *str, int hitnum, enum nmz_stat stat );

#endif /* _IDXNAME_H */
