#include <stdlib.h>
#include <errno.h>

#include "config-compat.h"

#ifndef HAVE_STRTONUM
long long strtonum(
    const char *nptr, long long minv, long long maxv, const char **errstr
) {
    char *err;
    long long ret = strtoll(nptr, &err, 10);
    if (*err) {
        errno = EINVAL;
        if (errstr) {
            *errstr = "invalid";
        }
        return 0;
    }
    if (ret < minv) {
        errno = ERANGE;
        if (errstr) {
            *errstr = "too small";
        }
        return 0;
    }
    if (ret > maxv) {
        errno = ERANGE;
        if (errstr) {
            *errstr = "too large";
        }
        return 0;
    }
    if (errstr) {
        *errstr = NULL;
    }
    return ret;
}
#endif
