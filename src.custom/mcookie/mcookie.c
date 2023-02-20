/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Daniel Kolesa
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/random.h>
#include <openssl/evp.h>

extern char const *__progname;

static struct option gnuopts[] = {
    {"verbose",     no_argument, NULL, 'v'},
    {"help",        no_argument, NULL, 'h'},
    {"version",     no_argument, NULL, 'V'},
    {0, 0, 0, 0}
};

#define RANDOM_BYTES 128

int main(int argc, char **argv) {
    int verbose = 0;
    int help = 0;
    int version = 0;

    for (;;) {
        int opt_idx = 0;
        int c = getopt_long(argc, argv, "+vhV", gnuopts, &opt_idx);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'v':
                verbose = 1;
                break;
            case 'h':
                help = 1;
                break;
            case 'V':
                version = 1;
                break;

            default:
                fprintf(stderr, "%s: invalid option -- '%c'\n", __progname, c);
                fprintf(
                    stderr, "Try '%s --help' for more information.\n",
                    __progname
                );
                return 1;
        }
    }

    if (help) {
        printf(
"Usage: %s [OPTION]...\n"
"\n"
"Generate magic cookies for xauth.\n"
"\n"
"      -v, --verbose  explain what is being done\n"
"      -h, --help     display this help and exit\n"
"      -V, --version  output version information and exit\n",
            __progname
        );
        return 0;
    } else if (version) {
        printf(
"%s (" PROJECT_NAME ") " PROJECT_VERSION "\n"
"Copyright (C) 2023 Daniel Kolesa\n"
"SPDX-License-Identifier: BSD-2-Clause\n",
            __progname
        );
        return 0;
    }

    char randbuf[RANDOM_BYTES];
    char mdbuf[RANDOM_BYTES / 4];
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();

    if (!ctx) {
        errx(1, "could not init context");
    }
    if (!EVP_DigestInit_ex(ctx, EVP_md5(), NULL)) {
        errx(1, "could not init digest");
    }

    /* try getrandom() first, nonblocking, good source */
    ssize_t ret = getrandom(
        randbuf, RANDOM_BYTES, GRND_RANDOM | GRND_NONBLOCK
    );
    if (ret == RANDOM_BYTES) {
        if (verbose) {
            fprintf(
                stderr, "Got %d bytes from getrandom() function\n", RANDOM_BYTES
            );
        }
        goto do_hash;
    }

    /* try /dev/urandom */
    int rfd = open("/dev/urandom", O_RDONLY);
    if (rfd >= 0) {
        ret = read(rfd, randbuf, RANDOM_BYTES);
        close(rfd);
        if (ret == RANDOM_BYTES) {
            if (verbose) {
                fprintf(stderr, "got %d bytes from /dev/urandom\n", RANDOM_BYTES);
            }
            goto do_hash;
        }
    }

    /* try libc PRNG as a fallback */
    {
        char *bufp = randbuf;
        char *ebuf = bufp + RANDOM_BYTES;
        srand(time(0));
        while (bufp <= ebuf) {
            int v = rand();
            size_t left = (ebuf - bufp);
            if (left < sizeof(v)) {
                memcpy(bufp, &v, left);
                break;
            } else {
                memcpy(bufp, &v, sizeof(v));
                bufp += sizeof(v);
            }
        }
        if (verbose) {
            fprintf(stderr, "got %d bytes from libc PRNG\n", RANDOM_BYTES);
        }
    }

do_hash:
    if (!EVP_DigestUpdate(ctx, randbuf, RANDOM_BYTES)) {
        errx(1, "could not update digest");
    }

    unsigned char digbuf[EVP_MAX_MD_SIZE + 1];
    unsigned int mdlen = 0;

    if (!EVP_DigestFinal(ctx, digbuf, &mdlen)) {
        errx(1, "could not finalize digest");
    }

    for (unsigned int i = 0; i < (sizeof(mdbuf) - 1); ++i) {
        sprintf(mdbuf + (i * 2), "%02x", digbuf[i]);
    }
    printf("%.*s\n", (int)sizeof(mdbuf), mdbuf);

    return 0;
}
