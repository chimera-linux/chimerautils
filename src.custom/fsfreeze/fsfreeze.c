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
#include <err.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/fs.h>

extern char const *__progname;

static struct option gnuopts[] = {
    {"freeze",             no_argument, NULL, 'f'},
    {"unfreeze",           no_argument, NULL, 'u'},
    {"help",               no_argument, NULL, 'h'},
    {"version",            no_argument, NULL, 'V'},
    {0, 0, 0, 0}
};

int main(int argc, char **argv) {
    int help = 0;
    int version = 0;
    unsigned long req = 0;

    for (;;) {
        int opt_idx = 0;
        int c = getopt_long(argc, argv, "fuhV", gnuopts, &opt_idx);
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

            case 'f':
                req = FIFREEZE;
                break;
            case 'u':
                req = FITHAW;
                break;

            default:
                fprintf(stderr, "%s: invalid option -- '%c'\n", __progname, c);
usage_help:
                fprintf(
                    stderr, "Try '%s --help' for more information.\n",
                    __progname
                );
                return 1;
        }
    }

    if (help) {
        printf(
"Usage: %s [OPTIONS]... MOUNTPOINT\n"
"\n"
"Suspend access to a filesystem.\n"
"\n"
"      -f, --freeze    freeze the filesystem\n"
"      -u, --unfreeze  unfreeze the filesystem\n"
"      -h, --help      display this help and exit\n"
"      -V, --version   output version information and exit\n",
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

    if ((req != FIFREEZE) && (req != FITHAW)) {
        errx(1, "you must specify either --freeze or --unfreeze");
    }

    if (argc == optind) {
        errx(1, "no mountpoint specified");
    } else if (argc != (optind + 1)) {
        fprintf(stderr, "%s: too many arguments\n", __progname);
        goto usage_help;
    }

    int fd = open(argv[optind], O_RDONLY);
    if (fd < 0) {
        err(1, "cannot open %s", argv[optind]);
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        err(1, "stat");
    }

    if (ioctl(fd, req, 0) < 0) {
        err(1, "ioctl");
    }

    return 0;
}
