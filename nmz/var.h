#ifndef _VARIABLE_H
#define _VARIABLE_H

extern int TfIdf;

extern char DEFAULT_INDEX[];

extern char BASE_URI[];
extern char CONFDIR[];
extern char NAMAZURC[];

extern struct nmz_replace *Replace;
extern struct nmz_alias   *Alias;

extern struct nmz_names NMZ;
extern struct nmz_files Nmz;
extern struct nmz_indices   Idx;
extern struct nmz_query     Query;

#endif /* _VARIABLE_H */
