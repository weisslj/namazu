#ifndef _CODECONV_H
#define _CODECONV_H

int conv_ja_any_to_eucjp(uchar *);
void zen2han(uchar *);
int ishiragana(uchar *);
int iskatakana(uchar *);
uchar *conv_ext(uchar *);

#endif /* _CODECONV_H */
