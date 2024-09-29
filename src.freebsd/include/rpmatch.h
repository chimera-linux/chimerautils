#ifndef _RPMATCH_H_
#define _RPMATCH_H_

#ifdef _CHIMERAUTILS_BUILD
#include "config-compat.h"
#endif

#if !defined(_CHIMERAUTILS_BUILD) || !defined(HAVE_RPMATCH)

#ifdef __cplusplus
extern "C" {
#endif

extern int rpmatch(const char *response);

#ifdef __cplusplus
}
#endif

#else
#  include <stdlib.h>
#endif

#endif
