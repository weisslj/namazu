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
extern int DbNumber;
extern int OppositeEndian;
extern int AllDocumentN;
extern int TfIdf;
extern int NoReference;

extern uchar KeyTable[];
extern uchar *KeyItem[];
extern uchar DEFAULT_DIR[];
extern uchar *DbNames[];
extern uchar Lang[];
extern FILE *Index, *IndexIndex;

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

NMZ_NAMES NMZ;
NMZ_FILES Nmz;

#endif /* _VARIABLE_H */
