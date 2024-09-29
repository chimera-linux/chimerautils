#ifndef _RPMATCH_H_
#define _RPMATCH_H_

#ifdef _CHIMERAUTILS_BUILD
#include "config-compat.h"
#endif

#if !defined(_CHIMERAUTILS_BUILD) || !defined(HAVE_RPMATCH)

extern int rpmatch(const char *response);

#endif

#endif
