#ifndef _CODECONV_H
#define _CODECONV_H

#define ASCII "\x1b(B"
#define NEWJIS_K "\x1b$B"

#define set1byte() fputs( ASCII, stdout )
#define set2byte() fputs( NEWJIS_K, stdout )

uchar jmstojis(uchar, uchar);
void jistoeuc(uchar *);
void sjistoeuc(uchar *);
void euctosjis(uchar *);
int codeconv(uchar *);
void zen2han(uchar *);
int ishiragana(uchar *);
int iskatakana(uchar *);

#endif /* _CODECONV_H */
