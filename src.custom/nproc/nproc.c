/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Daniel Kolesa
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
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/sysinfo.h>

extern const char *__progname;

static int opt_all, opt_help, opt_version;

static struct option gnuopts[] = {
    {"all",     no_argument,       &opt_all,     1},
    {"ignore",  required_argument, 0,            0},
    {"help",    no_argument,       &opt_help,    1},
    {"version", no_argument,       &opt_version, 1},
    {0, 0, 0, 0}
};

int main(int argc, char **argv) {
    int nignore = 0;
    int ncpus = 0;

    for (;;) {
        int opt_idx = 0;
        int c = getopt_long(argc, argv, "", gnuopts, &opt_idx);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 0:
                if (strcmp(gnuopts[opt_idx].name, "ignore")) {
                    continue;
                }
                nignore = atoi(optarg);
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

    if (opt_help) {
        printf(
"Usage: %s [OPTION]...\n"
"Print the number of processing units available to the current process,\n"
"which may be less than the number of online processors.\n"
"\n"
"      --all      print the number of installed processors\n"
"      --ignore=N  if possible, exclude N processing units\n"
"      --help     display this help and exit\n"
"      --version  output version information and exit\n",
            __progname
        );
        return 0;
    } else if (opt_version) {
        printf(
"%s (" PROJECT_NAME ") " PROJECT_VERSION "\n"
"Copyright (C) 2021 Daniel Kolesa\n"
"SPDX-License-Identifier: BSD-2-Clause\n",
            __progname
        );
        return 0;
    }

    if (!opt_all) {
        cpu_set_t cset;
        if (pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset)) {
            fprintf(stderr, "%s: pthread_getaffinity_np failed\n", __progname);
            return 1;
        }
        for (int i = 0; i < CPU_SETSIZE; ++i) {
            if (CPU_ISSET(i, &cset)) {
                ++ncpus;
            }
        }
    } else {
        ncpus = get_nprocs_conf();
    }

    if (nignore > 0) {
        if (nignore < ncpus) {
            ncpus -= nignore;
        } else {
            ncpus = 1;
        }
    }

    printf("%d\n", ncpus);

    return 0;
}
