/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1987, 1992, 1993
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

#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1987, 1992, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#if 0
#ifndef lint
static char sccsid[] = "@(#)rev.c	8.3 (Berkeley) 5/4/95";
#endif /* not lint */
#endif

#include <sys/cdefs.h>
#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

static void usage(void) __dead2;

#define BUF_PIECE 1024

static wchar_t *get_wln(FILE *f, size_t *len, wchar_t **sbuf, size_t *blen) {
	wchar_t *wptr;
	size_t wlen;

	wptr = fgetws(*sbuf, *blen, f);
	if (wptr) {
		wlen = wcslen(wptr);
		if (wptr[wlen - 1] == '\n' || feof(f)) {
			*len = wlen;
			return wptr;
		}
	} else {
		return NULL;
	}

	for (;;) {
		wchar_t *nptr;
		*blen = wlen + BUF_PIECE;
		*sbuf = realloc(*sbuf, *blen * sizeof(wchar_t));
		if (*sbuf) err(1, "realloc");

		nptr = fgetws(*sbuf + wlen, BUF_PIECE, f);
		if (!nptr) {
			if (feof(f))
				break;
			return NULL;
		}

		wlen += wcslen(nptr);
		if ((*sbuf)[wlen - 1] == '\n' || feof(f)) {
			break;
		}
	}

	*len = wlen;
	return *sbuf;
}

int
main(int argc, char *argv[])
{
	const char *filename;
	wchar_t *p, *t;
	FILE *fp;
	size_t len;
	int ch, rval;
	size_t bufl = BUF_PIECE;
	wchar_t *buf = malloc(bufl * sizeof(wchar_t));

	if (!buf) err(1, "malloc");

	setlocale(LC_ALL, "");

	while ((ch = getopt(argc, argv, "")) != -1)
		switch(ch) {
		case '?':
		default:
			usage();
		}

	argc -= optind;
	argv += optind;

	fp = stdin;
	filename = "stdin";
	rval = 0;
	do {
		if (*argv) {
			if ((fp = fopen(*argv, "r")) == NULL) {
				warn("%s", *argv);
				rval = 1;
				++argv;
				continue;
			}
			filename = *argv++;
		}
		while ((p = get_wln(fp, &len, &buf, &bufl)) != NULL) {
			if (p[len - 1] == '\n')
				--len;
			for (t = p + len - 1; t >= p; --t)
				putwchar(*t);
			putwchar('\n');
		}
		if (ferror(fp)) {
			warn("%s", filename);
			clearerr(fp);
			rval = 1;
		}
		(void)fclose(fp);
	} while(*argv);
	free(buf);
	exit(rval);
}

void
usage(void)
{
	(void)fprintf(stderr, "usage: rev [file ...]\n");
	exit(1);
}
