#ifndef _VARIABLE_H
#define _VARIABLE_H

extern int HListMax;
extern int HListWhence;
extern int Debug;
extern int ListFormat;
extern int Quiet;
extern int HitCountOnly;
extern int HtmlOutput;
extern int HidePageIndex;
extern int ForcePrintForm;
extern int AllList;
extern int FinalHitN;
extern int NoReplace;
extern int DecodeURI;
extern int IsCGI;
extern int Logging;
extern int TfIdf;
extern int NoReference;

extern int SortMethod;
extern int SortOrder;

extern uchar DEFAULT_INDEX[];

extern uchar BASE_URI[];
extern uchar CONFDIR[];
extern uchar NAMAZURC[];

extern uchar *ScriptName;
extern uchar *QueryString;
extern uchar *ContentLength;

extern uchar INDEX[];
extern uchar INDEXINDEX[];
extern uchar HEADERFILE[];
extern uchar FOOTERFILE[];
extern uchar LOCKFILE[];
extern uchar LOCKMSGFILE[];
extern uchar BODYMSGFILE[];
extern uchar RESULTFILE[];
extern uchar SLOG[];
extern uchar WORDLIST[];
extern uchar FIELDINFO[];
extern uchar DATEINDEX[];

extern uchar PHRASE[];
extern uchar PHRASEINDEX[];

extern REPLACE *Replace;
extern ALIAS   *Alias;

extern NMZ_NAMES NMZ;
extern NMZ_FILES Nmz;
extern INDICES   Idx;
extern QUERY     Query;
extern char      Template[];
extern char      Dyingmessage[];

#endif /* _VARIABLE_H */
