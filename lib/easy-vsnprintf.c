/* (v)snprintf in terms of __(v)snprintf
 *
 * Useful with Solaris 2.5 libc, which appears to have the `__*' versions
 * of (v)snprintf.
 *
 * Also useful with Win32 CRTDLL.DLL, which appears to have the `_*'
 * versions of (v)snprintf.
 *
 * This file is in the public domain
 * (in case it matters)
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdarg.h>
#include <stdlib.h>

#ifdef HAVE__VSNPRINTF
extern int _vsnprintf (char *, size_t, const char *, va_list);
#elif HAVE___VSNPRINTF
extern int __vsnprintf (char *, size_t, const char *, va_list);
#endif /* HAVE__VSNPRINTF || HAVE___VSNPRINTF */

int
vsnprintf (char *string, size_t maxlen, const char *format, va_list args)
{
#ifdef HAVE__VSNPRINTF
  return _vsnprintf (string, maxlen, format, args);
#elif HAVE___VSNPRINTF
  return __vsnprintf (string, maxlen, format, args);
#endif /* HAVE__VSNPRINTF || HAVE___VSNPRINTF */
}

int
snprintf (char *string, size_t maxlen, const char *format, ...)
{
  va_list args;
  int retval;
  va_start(args, format);
  retval = vsnprintf (string, maxlen, format, args);
  va_end(args);
  return retval;
}
