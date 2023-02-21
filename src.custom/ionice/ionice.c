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
#include <ctype.h>
#include <limits.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>
#include <sched.h>
#include <sys/syscall.h>
#include <linux/ioprio.h>

extern char const *__progname;
static int ignore = 0;

static struct option gnuopts[] = {
    {"classdata", required_argument, NULL, 'n'},
    {"class",     required_argument, NULL, 'c'},
    {"pid",       required_argument, NULL, 'p'},
    {"pgid",      required_argument, NULL, 'P'},
    {"uid",       required_argument, NULL, 'u'},
    {"ignore",          no_argument, NULL, 't'},
    {"help",            no_argument, NULL, 'h'},
    {"version",         no_argument, NULL, 'V'},
    {0, 0, 0, 0}
};

static void setid(int which, int who, int ioc, int data) {
    if (syscall(SYS_ioprio_set, which, who, IOPRIO_PRIO_VALUE(ioc, data)) < 0) {
        if (!ignore) {
            err(1, "ioprio_set failed");
        }
    }
}

static void print(int which, int who) {
    /* no options */
    int p = syscall(SYS_ioprio_get, which, who);
    if (p < 0) {
        err(1, "ioprio_get");
    }
    int cl = (p >> IOPRIO_CLASS_SHIFT) & IOPRIO_CLASS_MASK;
    char const *name = "unknown";
    switch (cl) {
        case IOPRIO_CLASS_NONE: name = "none"; break;
        case IOPRIO_CLASS_RT: name = "realtime"; break;
        case IOPRIO_CLASS_BE: name = "best-effort"; break;
        case IOPRIO_CLASS_IDLE: name = "idle"; break;
        default: break;
    }
    if (cl != IOPRIO_CLASS_IDLE) {
        printf("%s: prio %lu\n", name, p & IOPRIO_PRIO_MASK);
    } else {
        printf("%s\n", name);
    }
}

int main(int argc, char **argv) {
    int help = 0;
    int version = 0;
    int set = 0;
    long ioc = IOPRIO_CLASS_BE;
    long which = 0;
    int who = 0;
    unsigned long data = 4;
    char *end = NULL;
    char errarg = 0;

    for (;;) {
        int opt_idx = 0;
        int c = getopt_long(argc, argv, "+n:c:p:P:u:thV", gnuopts, &opt_idx);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'n':
                end = NULL;
                data = strtoul(optarg, &end, 10);
                if (!end || *end) {
                    errx(1, "invalid class data argument: %s", optarg);
                }
                set |= 1;
                break;

            case 'c':
                if (isdigit(*optarg)) {
                    end = NULL;
                    long v = strtol(optarg, &end, 10);
                    if (!end || *end || (v > INT_MAX) || (v < INT_MIN)) {
                        errx(1, "invalid class argument: %s", optarg);
                    }
                    ioc = (int)v;
                } else if (!strcasecmp(optarg, "none")) {
                    ioc = IOPRIO_CLASS_NONE;
                } else if (!strcasecmp(optarg, "realtime")) {
                    ioc = IOPRIO_CLASS_RT;
                } else if (!strcasecmp(optarg, "best-effort")) {
                    ioc = IOPRIO_CLASS_BE;
                } else if (!strcasecmp(optarg, "idle")) {
                    ioc = IOPRIO_CLASS_IDLE;
                } else {
                    errx(1, "invalid class argument: %s", optarg);
                }
                set |= 2;
                break;

            case 'p':
            case 'P':
            case 'u':
                if (who) {
                    errx(1, "specify one of -p, -P -u");
                }
                end = NULL;
                which = strtol(optarg, &end, 10);
                if (!end || *end || (which > INT_MAX) || (which < INT_MIN)) {
                    errx(1, "invalid -%c argument", c);
                }
                errarg = c;
                switch (c) {
                    case 'p': who = IOPRIO_WHO_PROCESS; break;
                    case 'P': who = IOPRIO_WHO_PGRP; break;
                    case 'u': who = IOPRIO_WHO_USER; break;
                    default: break;
                }
                break;

            case 't':
                ignore = 1;
                break;

            case 'h':
                help = 1;
                break;
            case 'V':
                version = 1;
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
"Usage: %s [OPTION]... -p PID...\n"
"       %s [OPTION]... -P PGID...\n"
"       %s [OPTION]... -u UID...\n"
"       %s [OPTION]... COMMAND [ARGS]...\n"
"\n"
"Show or change the I/O scheduling class and priority of a process.\n"
"\n"
"      -c, --class=CLASS    name or number of the scheduling class,\n"
"                           0: none, 1: realtime, 2: best-effort, 3: idle\n"
"      -n, --classdata=NUM  priority (0..7) in the specified scheduling\n"
"                           class, only for realtime and best-effort\n"
"      -p, --pid=PID...     act on these already running processes\n"
"      -P, --pgid=PGID...   act on already running processes in these groups\n"
"      -u, --uid=UID...     act on already running processes of these users\n"
"      -t, --ignore         ignore failures\n"
"      -h, --help           display this help and exit\n"
"      -V, --version        output version information and exit\n",
            __progname, __progname, __progname, __progname
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

    switch (ioc) {
        case IOPRIO_CLASS_NONE:
            if ((set & 1) && !ignore) {
                warnx("ignoring given class data for none class");
            }
            data = 0;
            break;
        case IOPRIO_CLASS_RT:
        case IOPRIO_CLASS_BE:
            break;
        case IOPRIO_CLASS_IDLE:
            if ((set & 1) && !ignore) {
                warnx("ignoring given class data for idle class");
            }
            data = 7;
            break;
        default:
            if (!ignore) {
                warnx("unknown priority class %ld", ioc);
            }
            break;
    }

    if (!set && !which && (optind == argc)) {
        /* no options */
        print(IOPRIO_WHO_PROCESS, 0);
        return 0;
    }

    if (who) {
        /* -p, -P, -u */
        if (!set) {
            print(who, (int)which);
        } else {
            setid(who, (int)which, ioc, data);
        }
        while (argv[optind]) {
            end = NULL;
            which = strtol(argv[optind], &end, 10);
            if (!end || *end || (which > INT_MAX) || (which < INT_MIN)) {
                errx(1, "invalid -%c argument", errarg);
            }
            if (!set) {
                print(who, (int)which);
            } else {
                setid(who, (int)which, ioc, data);
            }
            ++optind;
        }
        return 0;
    }

    if (!argv[optind]) {
        fprintf(stderr, "%s: bad usage\n", __progname);
        goto err_usage;
    }

    /* command */
    setid(IOPRIO_WHO_PROCESS, 0, ioc, data);
    execvp(argv[optind], &argv[optind]);
    err(1, "execvp");
    return 1;
}
