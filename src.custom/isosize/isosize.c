/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 q66 <q66@chimera-linux.org>
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
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <sys/stat.h>
#include <sys/types.h>

extern char const *__progname;

static struct option gnuopts[] = {
    {"divisor", required_argument, NULL, 'd'},
    {"sectors",       no_argument, NULL, 'x'},
    {"help",          no_argument, NULL, 'h'},
    {"version",       no_argument, NULL, 'V'},
    {0, 0, 0, 0}
};

static int xflg = 0;
static int icnt = 1;
static long divisor = 1;

static int is_iso(int fd) {
    char lbl[8];
    if (pread(fd, lbl, sizeof(lbl), 0x8000) < 0) {
        return 0;
    }
    return !memcmp(lbl, "\1CD001\1", sizeof(lbl));
}

static int isosize(char const *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        warn("%s: open", path);
        return 0;
    }

    if (!is_iso(fd)) {
        warnx("%s: not an ISO9660 filesystem", path);
        close(fd);
        return 0;
    }

    char vss[4], lbs[2];
    errno = 0;
    if (
        (pread(fd, vss, sizeof(vss), 0x8050) != sizeof(vss)) ||
        (pread(fd, lbs, sizeof(lbs), 0x8080) != sizeof(lbs))
    ) {
        if (errno) {
            warn("%s: read error", path);
        } else {
            warnx("%s: read error", path);
        }
        close(fd);
        return 0;
    }

    close(fd);

    unsigned int nsecs = (
        ((vss[0] & 0xffu)) |
        ((vss[1] & 0xffu) << 8) |
        ((vss[2] & 0xffu) << 16) |
        ((vss[3] & 0xffu) << 24)
    );
    unsigned int ssize = ((lbs[0] & 0xffu) | ((lbs[1] & 0xffu) << 8));

    if (icnt > 1) {
        printf("%s: ", path);
    }
    if (xflg) {
        printf("sector count: %u, sector size: %u\n", nsecs, ssize);
    } else {
        printf("%lld\n", ((long long)nsecs * ssize) / divisor);
    }

    return 1;
}

int main(int argc, char **argv) {
    int help = 0;
    int version = 0;

    for (;;) {
        int opt_idx = 0;
        int c = getopt_long(argc, argv, "d:xhV", gnuopts, &opt_idx);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'h':
                help = 1;
                break;
            case 'V':
                version = 1;
                break;

            case 'd': {
                char *err = NULL;
                divisor = strtol(optarg, &err, 10);
                if (!err || *err || !divisor) {
                    errx(1, "invalid divisor argument");
                }
                break;
            }

            case 'x':
                xflg = 1;
                break;

            default:
                fprintf(stderr, "%s: invalid option -- '%c'\n", __progname, c);
err_usage:
                fprintf(
                    stderr, "Try '%s --help' for more information.\n",
                    __progname
                );
                return 1;
        }
    }

    if (help) {
        printf(
"Usage: %s [OPTIONS]... IMAGE...\n"
"\n"
"Show the length of an ISO9660 filesystem.\n"
"\n"
"      -d, --divisor=NUM  divide the amount of bytes by NUM\n"
"      -x, --sectors      show sector count and size\n"
"      -h, --help         display this help and exit\n"
"      -V, --version      output version information and exit\n",
            __progname
        );
        return 0;
    } else if (version) {
        printf(
"%s (" PROJECT_NAME ") " PROJECT_VERSION "\n"
"Copyright (C) 2023 q66 <q66@chimera-linux.org>\n"
"SPDX-License-Identifier: BSD-2-Clause\n",
            __progname
        );
        return 0;
    }

    icnt = (argc - optind);

    if (icnt < 1) {
        fprintf(stderr, "%s: no device specified\n", __progname);
        goto err_usage;
    }

    int nerr = 0;
    for (int i = optind; i < argc; ++i) {
        if (!isosize(argv[i])) {
            ++nerr;
        }
    }

    return ((icnt == nerr) ? 32 : (nerr ? 64 : 0));
}
