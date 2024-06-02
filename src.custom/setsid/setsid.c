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
#include <err.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

extern char const *__progname;

static struct option gnuopts[] = {
    {"ctty",    no_argument, NULL, 'c'},
    {"fork",    no_argument, NULL, 'f'},
    {"wait",    no_argument, NULL, 'w'},
    {"help",    no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'V'},
    {0, 0, 0, 0}
};

int main(int argc, char **argv) {
    int help = 0;
    int version = 0;
    int ctty = 0;
    int dofork = 0;
    int dowait = 0;

    for (;;) {
        int opt_idx = 0;
        int c = getopt_long(argc, argv, "+cfwhV", gnuopts, &opt_idx);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'c':
                ctty = 1;
                break;
            case 'f':
                dofork = 1;
                break;
            case 'w':
                dowait = 1;
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
"Usage: %s [OPTION]... <PROGRAM> [ARGUMENTS]...\n"
"\n"
"Run a program in a new session.\n"
"\n"
"      -c, --ctty     set the controling terminal to the current one\n"
"      -f, --fork     always fork\n"
"      -w, --wait     wait for program to exit and return the same code\n"
"      -h, --help     display this help and exit\n"
"      -V, --version  output version information and exit\n",
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

    if ((argc - optind) < 1) {
        errx(1, "no command specified");
    }

    if (dofork || (getpgrp() == getpid())) {
        pid_t fpid = fork();
        if (fpid < 0) {
            err(1, "fork");
        }
        if (fpid > 0) {
            int status;
            if (!dowait) {
                return 0;
            }
            if (wait(&status) != fpid) {
                err(1, "wait");
            }
            if (WIFEXITED(status)) {
                return WEXITSTATUS(status);
            }
            err(status, "child process %ld exited unexpectedly", (long)fpid);
        }
        /* child */
    }

    if (setsid() < 0) {
        err(1, "setsid");
    }

    if (ctty && ioctl(STDIN_FILENO, TIOCSCTTY, 1)) {
        err(1, "ioctl");
    }

    execvp(argv[optind], argv + optind);
    err(1, "execvp");

    return 1;
}
