#ifndef _VARIABLE_H
#define _VARIABLE_H

extern int HListMax;
extern int HListWhence;
extern int Debug;
extern int ShortFormat;
extern int MoreShortFormat;
extern int Quiet;
extern int HitCountOnly;
extern int ScoreSort;
extern int HtmlOutput;
extern int HidePageIndex;
extern int ForcePrintForm;
extern int AllList;
extern int LaterOrder;
extern int FinalHitN;
extern int ConfLoaded;
extern int NoReplace;
extern int DecodeURI;
extern int IsCGI;
extern int Logging;
extern int OppositeEndian;
extern int TfIdf;
extern int NoReference;

extern uchar DEFAULT_DIR[];
extern uchar Lang[];

extern uchar BASE_URI[];
extern uchar NAMAZU_CONF[];
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

extern REPLACE Replace;
extern ALIAS   *Alias;

extern NMZ_NAMES NMZ;
extern NMZ_FILES Nmz;
extern INDICES   Idx;
extern QUERY     Query;

#endif /* _VARIABLE_H */
