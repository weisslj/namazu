#ifndef _CODECONV_H
#define _CODECONV_H

extern int conv_ja_any_to_eucjp(char *);
extern void zen2han(char *);
extern int ishiragana(char *);
extern int iskatakana(char *);
extern char *conv_ext(char *);

#endif /* _CODECONV_H */
