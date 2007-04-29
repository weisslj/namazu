#ifndef _CODECONV_H
#define _CODECONV_H

extern char *nmz_codeconv_external(const char *);
extern void nmz_codeconv_query ( char *query );

extern char *nmz_from_to(char *buffer, int bufferSize, const char *fromCode, const char *toCode);
extern char *nmz_codeconv(const char *fromCode, char *fromBuffer, int fromBufferSize, const char *toCode, char *toBuffer, int toBufferSize);
extern int nmz_lengthEUCJP(const char *str, int length);
extern char *nmz_get_external_encoding(void);

#endif /* _CODECONV_H */
