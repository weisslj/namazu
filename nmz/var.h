#ifndef _VARIABLE_H
#define _VARIABLE_H

extern int TfIdf;

extern char DEFAULT_INDEX[];

extern char BASE_URI[];
extern char CONFDIR[];
extern char NAMAZURC[];

extern char INDEX[];
extern char INDEXINDEX[];
extern char HEADERFILE[];
extern char FOOTERFILE[];
extern char LOCKFILE[];
extern char LOCKMSGFILE[];
extern char BODYMSGFILE[];
extern char RESULTFILE[];
extern char SLOG[];
extern char WORDLIST[];
extern char FIELDINFO[];
extern char DATEINDEX[];

extern char PHRASE[];
extern char PHRASEINDEX[];

extern REPLACE *Replace;
extern ALIAS   *Alias;

extern NMZ_NAMES NMZ;
extern NMZ_FILES Nmz;
extern INDICES   Idx;
extern QUERY     Query;

#endif /* _VARIABLE_H */
