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
#include <limits.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>
#include <sched.h>
#include <sys/syscall.h>

extern char const *__progname;

static struct option gnuopts[] = {
    {"all-tasks",   no_argument, NULL, 'a'},
    {"pid",         no_argument, NULL, 'p'},
    {"cpu-list",    no_argument, NULL, 'c'},
    {"help",        no_argument, NULL, 'h'},
    {"version",     no_argument, NULL, 'V'},
    {0, 0, 0, 0}
};

static int cpulist = 0;

static void tprint(cpu_set_t *set, size_t ssize) {
    int prev = 0;
    if (cpulist) {
        for (size_t i = 0; i < (8 * ssize); ++i) {
            if (CPU_ISSET_S(i, ssize, set)) {
                size_t rnum = 0;
                /* guess a range */
                for (size_t j = i + 1; j < (8 * ssize); ++j) {
                    if (CPU_ISSET_S(j, ssize, set)) {
                        ++rnum;
                    } else {
                        break;
                    }
                }
                if (prev) {
                    printf(",");
                }
                if (!rnum) {
                    /* not a range */
                    printf("%zu", i);
                } else if (rnum == 1) {
                    /* could skip this branch but while at it */
                    printf("%zu,%zu", i, i + 1);
                    i += 1;
                } else {
                    /* range */
                    printf("%zu-%zu", i, i + rnum);
                    i += rnum;
                }
                prev = 1; /* start printing commas */
            }
        }
    } else {
        int cpu;
        for (cpu = (8 * ssize) - 4; cpu >= 0; cpu -= 4) {
            char val = 0;
            if (CPU_ISSET_S(cpu, ssize, set)) {
                val |= 0x1;
            }
            if (CPU_ISSET_S(cpu + 1, ssize, set)) {
                val |= 0x2;
            }
            if (CPU_ISSET_S(cpu + 2, ssize, set)) {
                val |= 0x4;
            }
            if (CPU_ISSET_S(cpu + 3, ssize, set)) {
                val |= 0x8;
            }
            if (val || prev) {
                printf("%x", val);
                prev = 1;
            }
        }
    }
}

static void taskset(long pid, cpu_set_t *gset, cpu_set_t *sset, size_t ssize) {
    char const *nm = (cpulist ? "list" : "mask");

    if (pid) {
        if (sched_getaffinity((pid_t)pid, ssize, gset) < 0) {
            err(1, "failed to get affinity for %ld", pid);
        }
        printf("pid %ld's current affinity %s: ", pid, nm);
        tprint(gset, ssize);
        printf("\n");
    }
    if (!sset) {
        return;
    }
    if (sched_setaffinity((pid_t)pid, ssize, sset) < 0) {
        err(1, "failed to set affinity for %ld", pid);
    }
    if (pid) {
        if (sched_getaffinity((pid_t)pid, ssize, gset) < 0) {
            err(1, "failed to get affinity for %ld", pid);
        }
        printf("pid %ld's new affinity %s: ", pid, nm);
        tprint(gset, ssize);
        printf("\n");
    }
}

int main(int argc, char **argv) {
    int help = 0;
    int version = 0;
    int all = 0;
    long pid = 0;

    for (;;) {
        int opt_idx = 0;
        int c = getopt_long(argc, argv, "+apchV", gnuopts, &opt_idx);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'a':
                all = 1;
                break;

            case 'p': {
                char *errp = NULL;
                pid = strtol(argv[argc - 1], &errp, 10);
                if (!errp || *errp) {
                    errx(1, "invalid pid value");
                }
                break;
            }

            case 'c':
                cpulist = 1;
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
"Usage: %s [OPTION]... [MASK|CPULIST] [PID|CMD [ARG]...]\n"
"\n"
"Show or change the CPU affinity of a process.\n"
"\n"
"      -a, --all-tasks  operate on all tasks/threads for the given PID\n"
"      -p, --pid        operate on an existing PID\n"
"      -c, --cpu-list   display/specify CPUs in a list format\n"
"      -h, --help       display this help and exit\n"
"      -V, --version    output version information and exit\n",
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

    int neargs = (argc - optind);

    if ((!pid && (neargs < 2)) || (pid && ((neargs < 1) || (neargs > 2)))) {
        fprintf(stderr, "%s: bad usage\n", __progname);
        goto err_usage;
    }

    cpu_set_t *gset, *sset = NULL;

    /* determine maximum number of cpus */
    int maxcpus;
    size_t setsize;
    {
        int cpus = 2048;
        /* allocate initial affinity mask */
        gset = CPU_ALLOC(cpus);
        if (!gset) {
            err(1, "CPU_ALLOC");
        }
        setsize = CPU_ALLOC_SIZE(cpus);

        for (;;) {
            CPU_ZERO_S(setsize, gset);
            /* use the raw syscall as it returns the number */
            maxcpus = syscall(SYS_sched_getaffinity, 0, setsize, gset);
            /* in case of failure, our affinity mask is too small */
            if ((maxcpus < 0) && (errno == EINVAL) && (cpus < (1024 * 1024))) {
                CPU_FREE(gset);
                /* in which case, double its size */
                cpus *= 2;
                gset = CPU_ALLOC(cpus);
                if (!gset) {
                    err(1, "CPU_ALLOC");
                }
                setsize = CPU_ALLOC_SIZE(cpus);
                /* and try again */
                continue;
            }
            /* no failure, the number is sufficient */
            CPU_FREE(gset);
            break;
        }
    }

    /* same number for both cpu sets */
    setsize = CPU_ALLOC_SIZE(maxcpus);
    /* for getaffinity */
    gset = CPU_ALLOC(maxcpus);
    if (!gset) {
        err(1, "CPU_ALLOC");
    }
    /* for setaffinity */
    if (neargs > 1) {
        sset = CPU_ALLOC(maxcpus);
        if (!sset) {
            err(1, "CPU_ALLOC");
        }
        CPU_ZERO_S(setsize, sset);
    } else {
        /* we can only be get-only with pid and no mask/list */
        goto do_taskset;
    }

    char *s = argv[optind];

    if (cpulist) {
        /* parse input list */
        for (;;) {
            /* parse the first number */
            char *end = NULL;
            unsigned long a = strtoul(s, &end, 10);
            if (!end || (end == s)) {
                /* could not parse a number */
                errx(1, "could not parse cpu list");
            }
            /* are we a range? */
            if (*end == '-') {
                s = end + 1;
                end = NULL;
                unsigned long b = strtoul(s, &end, 10);
                if (!end || (end == s)) {
                    errx(1, "could not parse cpu list");
                }
                /* a must be lower or same than b */
                if (a > b) {
                    errx(1, "invalid cpu range");
                }
                /* we are, maybe check for stride too */
                unsigned long stride = 1;
                if (*end == ':') {
                    s = end + 1;
                    end = NULL;
                    stride = strtoul(s, &end, 10);
                    if (!end || (end == s) || !stride) {
                        errx(1, "could not parse cpu list");
                    }
                }
                /* adjust the cpu set */
                while (a <= b) {
                    CPU_SET_S(a, setsize, sset);
                    a += stride;
                }
            } else {
                /* not a range, just a number */
                CPU_SET_S(a, setsize, sset);
            }
            /* end */
            if (!*end) {
                break;
            }
            /* the list continues, skip comma */
            if (*end == ',') {
                s = end + 1;
            }
        }
    } else {
        /* parse input mask, which is always hex; first skip potential 0x */
        if (!strncmp(s, "0x", 2)) {
            s += 2;
        }
        /* we need to parse from the end */
        size_t mlen = strlen(s);
        char *e = s + mlen - 1;
        int cpu = 0;
        while (e >= s) {
            if (*e == ',') {
                /* sysfs masks */
                --e;
                continue;
            }
            /* lowercasify */
            char c = *e | 32;
            char v;
            if ((c >= '0') && (c <= '9')) {
                v = (c - '0');
            } else if ((c >= 'a') && (c <= 'f')) {
                v = (c - 'a') + 10;
            } else {
                errx(1, "invalid mask format");
            }
            /* set */
            if (v & 0x1) {
                CPU_SET_S(cpu, setsize, sset);
            }
            if (v & 0x2) {
                CPU_SET_S(cpu + 1, setsize, sset);
            }
            if (v & 0x4) {
                CPU_SET_S(cpu + 2, setsize, sset);
            }
            if (v & 0x8) {
                CPU_SET_S(cpu + 3, setsize, sset);
            }
            --e;
            cpu += 4;
        }
    }

do_taskset:
    if (all && pid) {
        char buf[PATH_MAX];
        snprintf(buf, sizeof(buf), "/proc/%ld/task", pid);
        DIR *dp = opendir(buf);
        if (!dp) {
            err(1, "failed to open procfs for %ld", pid);
        }
        struct dirent *d;
        while ((d = readdir(dp))) {
            char *endp = NULL;
            if (d->d_type != DT_DIR) {
                continue;
            }
            pid = strtol(d->d_name, &endp, 10);
            if (!endp || *endp) {
                continue;
            }
            taskset(pid, gset, sset, setsize);
        }
        closedir(dp);
    } else {
        taskset(pid, gset, sset, setsize);
    }

    CPU_FREE(gset);
    CPU_FREE(sset);

    if (!pid) {
        execvp(argv[optind + 1], argv + optind + 1);
        err(1, "execvp");
    }

    return 0;
}
