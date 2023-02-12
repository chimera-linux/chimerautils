#include <err.h>
#include <stdarg.h>
#include <errno.h>

#ifndef HAVE_ERRC
void errc(int eval, int code, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    errno = code;
    verr(eval, fmt, ap);
    va_end(ap);
}
#endif

#ifndef HAVE_WARNC
void warnc(int code, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    errno = code;
    vwarn(fmt, ap);
    va_end(ap);
}
#endif
