#ifndef _CODECONV_H
#define _CODECONV_H

extern int nmz_conv_ja_any_to_eucjp(char *);
extern void nmz_zen2han(char *);
extern int nmz_ishiragana(const char *);
extern int nmz_iskatakana(const char *);
extern char *nmz_conv_ext(const char *);

#endif /* _CODECONV_H */
