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

#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <getopt.h>
#include <sysexits.h>
#include <paths.h>
#include <fcntl.h>
#include <err.h>

extern char const *__progname;

static struct option gnuopts[] = {
    {"shared",      no_argument, NULL, 's'},
    {"exclusive",   no_argument, NULL, 'x'},
    {"unlock",      no_argument, NULL, 'u'},
    {"nonblocking", no_argument, NULL, 'n'},
    {"nb",          no_argument, NULL, 'n'},
    {"no-fork",     no_argument, NULL, 'F'},
    {"help",        no_argument, NULL, 'h'},
    {"version",     no_argument, NULL, 'V'},
    {0, 0, 0, 0}
};

static int open_f(char const *fname, int *flags) {
    int fl = (!*flags ? O_RDONLY : *flags) | O_NOCTTY | O_CREAT;
    int fd = open(fname, fl, 0666);
    if ((fd < 0) && (errno == EISDIR)) {
        fl = O_RDONLY | O_NOCTTY;
        fd = open(fname, fl);
    }
    if (fd < 0) {
        err((
            (errno == ENOMEM) || (errno == EMFILE) || (errno == ENFILE)
        ) ? EX_OSERR : (
            ((errno == EROFS) || (errno == ENOSPC)) ? EX_CANTCREAT : EX_NOINPUT
        ), "cannot open lock file %s", fname);
    }
    *flags = fl;
    return fd;
}

int main(int argc, char **argv) {
    char const *fname = NULL;
    pid_t fpid;
    int exstatus = 0;
    int type = LOCK_EX;
    int block = 0;
    int do_fork = 1;
    int oflags = 0;
    int fd = -1;
    int help = 0;
    int version = 0;
    char **cargv = NULL;
    char *sargv[4];

    for (;;) {
        int opt_idx = 0;
        int c = getopt_long(argc, argv, "+sexunFhV", gnuopts, &opt_idx);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 's':
                type = LOCK_SH;
                break;
            case 'e':
            case 'x':
                type = LOCK_EX;
                break;
            case 'u':
                type = LOCK_UN;
                break;
            case 'n':
                block = LOCK_NB;
                break;
            case 'F':
                do_fork = 0;
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
                return EX_USAGE;
        }
    }

    if (help) {
        printf(
"Usage: %s [OPTION]... <file>|<directory> <command> [<argument>...]\n"
"       %s [OPTION]... <file>|<directory> -c <command>\n"
"       %s [OPTION]... <file descriptor number>\n"
"\n"
"Manage file locks from shell scripts.\n"
"\n"
"      -s, --shared       get a shared lock\n"
"      -x, --exclusive    get an exclusive lock (default)\n"
"      -u, --unlock       remove a lock\n"
"      -n, --nonblocking  fail rather than wait\n"
"      -F, --no-fork      execute command without forking\n"
"      -h, --help         display this help and exit\n"
"      -V, --version      output version information and exit\n",
            __progname, __progname, __progname
        );
        return EX_OK;
    } else if (version) {
        printf(
"%s (" PROJECT_NAME ") " PROJECT_VERSION "\n"
"Copyright (C) 2023 Daniel Kolesa\n"
"SPDX-License-Identifier: BSD-2-Clause\n",
            __progname
        );
        return EX_OK;
    }

    if (argc > (optind + 1)) {
        if (
            !strcmp(argv[optind + 1], "-c") ||
            !strcmp(argv[optind + 1], "--command")
        ) {
            if (argc != (optind + 3)) {
                errx(EX_USAGE, "one command is required");
            }
            cargv = sargv;
            sargv[0] = getenv("SHELL");
            if (!sargv[0] || !*sargv[0]) {
                sargv[0] = _PATH_BSHELL;
            }
            sargv[1] = "-c";
            sargv[2] = argv[optind + 2];
            sargv[3] = NULL;
        } else {
            cargv = argv + optind + 1;
        }
        fname = argv[optind];
        errno = 0;
        fd = open_f(fname, &oflags);
    } else if (argc == (optind + 1)) {
        char *endp = NULL;
        unsigned long n = strtoul(argv[optind], &endp, 10);
        if (!endp || *endp || (n > INT_MAX) || fcntl((int)n, F_GETFD) < 0) {
            errx(EXIT_FAILURE, "invalid file descriptor: %s", argv[optind]);
        }
        fd = (int)n;
    } else {
        errx(EX_USAGE, "path or file descriptor is required");
    }

    while (flock(fd, type | block)) {
        switch (errno) {
            case EWOULDBLOCK:
                return EXIT_FAILURE;
            case EINTR:
                continue;
            case EIO:
            case EBADF:
                /* util-linux: probably emulated nfsv4 flock */
                if (
                    !(oflags & O_RDWR) && (type != LOCK_SH) &&
                    fname && (access(fname, R_OK | W_OK) == 0)
                ) {
                    close(fd);
                    oflags = O_RDWR;
                    errno = 0;
                    fd = open_f(fname, &oflags);
                    if (oflags & O_RDWR) {
                        break;
                    }
                }
                /* FALLTHROUGH */
            default:
                if (fname) {
                    warn("%s", fname);
                } else {
                    warn("%d", fd);
                }
                if (((errno == ENOLCK) || (errno == ENOMEM))) {
                    return EX_OSERR;
                }
                return EX_DATAERR;
        }
    }

    if (!cargv) {
        return EX_OK;
    }

    signal(SIGCHLD, SIG_DFL);

    if (!do_fork) {
        goto do_exec;
    }

    fpid = fork();

    if (fpid < 0) {
        err(EX_OSERR, "fork failed");
    } else if (fpid == 0) {
        /* child */
        goto do_exec;
    }

    /* parent */
    for (;;) {
        pid_t wpid = waitpid(fpid, &exstatus, 0);
        if (wpid < 0) {
            if (errno == EINTR) {
                continue;
            }
            err(EXIT_FAILURE, "waitpid failed");
        } else if (wpid == fpid) {
            break;
        }
    }

    if (WIFEXITED(exstatus)) {
        return WEXITSTATUS(exstatus);
    }
    if (WIFSIGNALED(exstatus)) {
        return WTERMSIG(exstatus) + 128;
    }
    return EX_OSERR;

do_exec:
    execvp(cargv[0], cargv);
    warn("failed to execute %s", cargv[0]);
    return ((errno == ENOMEM) ? EX_OSERR : EX_UNAVAILABLE);
}
