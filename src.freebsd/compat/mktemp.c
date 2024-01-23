/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1987, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)mktemp.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/random.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

char *_mktemp(char *);

static int _gettemp(int, char *, int *, int, int);

static const unsigned char padchar[] =
"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

int
compat_mkostemp(char *path, int oflags)
{
	int fd;

	return (_gettemp(AT_FDCWD, path, &fd, 0, oflags) ? fd : -1);
}

int
compat_mkstemp(char *path)
{
	int fd;

	return (_gettemp(AT_FDCWD, path, &fd, 0, 0) ? fd : -1);
}

char *
compat_mkdtemp(char *path)
{
	return (_gettemp(AT_FDCWD, path, (int *)NULL, 1, 0) ? path : (char *)NULL);
}

static int
_gettemp(int dfd, char *path, int *doopen, int domkdir, int oflags)
{
	char *start, *trv, *suffp, *carryp;
	char *pad;
	struct stat sbuf;
	char carrybuf[MAXPATHLEN];
	int saved;

	if ((doopen != NULL && domkdir) ||
	    (oflags & ~(O_APPEND | O_DIRECT | O_SYNC | O_CLOEXEC)) != 0) {
		errno = EINVAL;
		return (0);
	}

	trv = path + strlen(path);
	if (trv - path >= MAXPATHLEN) {
		errno = ENAMETOOLONG;
		return (0);
	}
	suffp = trv;
	--trv;
	if (trv < path) {
		errno = EINVAL;
		return (0);
	}

	/* Fill space with random characters */
	if (*trv == 'X') {
		char *bx = trv;
		while (bx > path && *(bx - 1) == 'X') --bx;
		if ((suffp - bx) > 256) {
			errno = EINVAL;
			return (0);
		}
		if (getrandom(bx, suffp - bx, GRND_NONBLOCK) < 0) {
			/* fall back to crappy randomness */
			struct timespec ts;
			uint64_t seed;
			clock_gettime(CLOCK_REALTIME, &ts);
			seed = ts.tv_sec + ts.tv_nsec + gettid() * 65537UL - 1;
			for (char *buf = bx; buf < suffp; buf += sizeof(seed)) {
				size_t left = (suffp - buf);
				seed = 6364136223846793005ULL * seed + 1;
				memcpy(buf, &seed, (left > sizeof(seed)) ? sizeof(seed) : left);
			}
		}
		start = bx;
		while (bx < suffp) {
			*bx = padchar[*bx % sizeof(padchar)];
			++bx;
		}
	} else start = trv + 1;

	saved = 0;
	oflags |= O_CREAT | O_EXCL | O_RDWR;
	for (;;) {
		if (doopen) {
			*doopen = openat(dfd, path, oflags, 0600);
			if (*doopen >= 0)
				return (1);
			if (errno != EEXIST)
				return (0);
		} else if (domkdir) {
			if (mkdir(path, 0700) == 0)
				return (1);
			if (errno != EEXIST)
				return (0);
		} else if (lstat(path, &sbuf))
			return (errno == ENOENT);

		/* save first combination of random characters */
		if (!saved) {
			memcpy(carrybuf, start, suffp - start);
			saved = 1;
		}

		/* If we have a collision, cycle through the space of filenames */
		for (trv = start, carryp = carrybuf;;) {
			/* have we tried all possible permutations? */
			if (trv == suffp)
				return (0); /* yes - exit with EEXIST */
			pad = strchr((char *)padchar, *trv);
			if (pad == NULL) {
				/* this should never happen */
				errno = EIO;
				return (0);
			}
			/* increment character */
			*trv = (*++pad == '\0') ? padchar[0] : *pad;
			/* carry to next position? */
			if (*trv == *carryp) {
				/* increment position and loop */
				++trv;
				++carryp;
			} else {
				/* try with new name */
				break;
			}
		}
	}
	/*NOTREACHED*/
}
