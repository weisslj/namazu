#ifndef _CODECONV_H
#define _CODECONV_H

extern int nmz_codeconv_internal(char *);
extern char *nmz_codeconv_external(const char *);
extern void nmz_codeconv_query ( char *query );

extern char *nmz_from_to(char *buffer, int bufferSize, const char *fromCode, const char *toCode);
extern char *nmz_codeconv(const char *fromCode, char *fromBuffer, int fromBufferSize, const char *toCode, char *toBuffer, int toBufferSize);
extern int nmz_lengthEUCJP(const char *str, int length);

#endif /* _CODECONV_H */
