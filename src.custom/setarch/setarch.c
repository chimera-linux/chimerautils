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

#include <sys/personality.h>
#include <sys/utsname.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <paths.h>
#include <err.h>

extern char const *__progname;

enum {
    LONGOPT_4GB = 256,
    LONGOPT_UNAME26,
    LONGOPT_LIST,
};

#ifndef UNAME26
#define UNAME26 0x0020000
#endif
#ifndef FDPIC_FUNCPTRS
#define FDPIC_FUNCPTRS 0x0080000
#endif

static struct option gnuopts[] = {
    {"addr-no-normalize",  no_argument, NULL, 'R'},
    {"fdpic-funcptrs",     no_argument, NULL, 'F'},
    {"mmap-page-zero",     no_argument, NULL, 'Z'},
    {"addr-compat-layout", no_argument, NULL, 'L'},
    {"read-implies-exec",  no_argument, NULL, 'X'},
    {"32bit",              no_argument, NULL, 'B'},
    {"short-inode",        no_argument, NULL, 'I'},
    {"whole-seconds",      no_argument, NULL, 'S'},
    {"sticky-timeouts",    no_argument, NULL, 'T'},
    {"3gb",                no_argument, NULL, '3'},
    {"4gb",                no_argument, NULL, LONGOPT_4GB},
    {"uname-2.6",          no_argument, NULL, LONGOPT_UNAME26},
    {"list",               no_argument, NULL, LONGOPT_LIST},
    {"verbose",            no_argument, NULL, 'v'},
    {"help",               no_argument, NULL, 'h'},
    {"version",            no_argument, NULL, 'V'},
    {0, 0, 0, 0}
};

/* matches util-linux, for compatibility */
static struct domain {
    int pval;
    char const *tarch;
    char const *rarch;
} domains[] = {
    {UNAME26,     "uname26", NULL},
    {PER_LINUX32, "linux32", NULL},
    {PER_LINUX,   "linux64", NULL},
#if defined(__arm__) || defined(__aarch64__)
#if defined(__BIG_ENDIAN__)
    {PER_LINUX32, "armv7b", "arm"},
    {PER_LINUX32, "armv8b", "arm"},
#else
    {PER_LINUX32, "armv7l", "arm"},
    {PER_LINUX32, "armv8l", "arm"},
#endif
    {PER_LINUX32, "armh", "arm"},
    {PER_LINUX32, "arm", "arm"},
    {PER_LINUX, "arm64", "aarch64"},
    {PER_LINUX, "aarch64", "aarch64"},
#endif
#if defined(__alpha__)
    {PER_LINUX, "alpha", "alpha"},
    {PER_LINUX, "alphaev5", "alpha"},
    {PER_LINUX, "alphaev56", "alpha"},
    {PER_LINUX, "alphaev6", "alpha"},
    {PER_LINUX, "alphaev67", "alpha"},
#endif
#if defined(__e2k__)
    {PER_LINUX, "e2k", "e2k"},
    {PER_LINUX, "e2kv4", "e2k"},
    {PER_LINUX, "e2kv5", "e2k"},
    {PER_LINUX, "e2kv6", "e2k"},
    {PER_LINUX, "e2k4c", "e2k"},
    {PER_LINUX, "e2k8c", "e2k"},
    {PER_LINUX, "e2k1cp", "e2k"},
    {PER_LINUX, "e2k8c2", "e2k"},
    {PER_LINUX, "e2k12c", "e2k"},
    {PER_LINUX, "e2k16c", "e2k"},
    {PER_LINUX, "e2k2c3", "e2k"},
#endif
#if defined(__hppa__)
    {PER_LINUX32, "parisc32", "parisc"},
    {PER_LINUX32, "parisc", "parisc"},
    {PER_LINUX, "parisc64", "parisc64"},
#endif
#if defined(__ia64__) || defined(__i386__)
    {PER_LINUX, "ia64", "ia64"},
#endif
#if defined(__mips64__) || defined(__mips__)
    {PER_LINUX32, "mips32", "mips"},
    {PER_LINUX32, "mips", "mips"},
    {PER_LINUX, "mips64", "mips64"},
#endif
#if defined(__powerpc__) || defined(__powerpc64__)
#if defined(__BIG_ENDIAN__)
    {PER_LINUX32, "ppc32", "ppc"},
    {PER_LINUX32, "ppc", "ppc"},
    {PER_LINUX, "ppc64", "ppc64"},
    {PER_LINUX, "ppc64pseries", "ppc64"},
    {PER_LINUX, "ppc64iseries", "ppc64"},
#else
    {PER_LINUX32, "ppc32", "ppcle"},
    {PER_LINUX32, "ppc", "ppcle"},
    {PER_LINUX32, "ppc32le", "ppcle"},
    {PER_LINUX32, "ppcle", "ppcle"},
    {PER_LINUX, "ppc64le", "ppc64le"},
#endif
#endif
#if defined(__s390x__) || defined(__s390__)
    {PER_LINUX32, "s390", "s390"},
    {PER_LINUX, "s390x", "s390x"},
#endif
#if defined(__sparc64__) || defined(__sparc__)
    {PER_LINUX32, "sparc", "sparc"},
    {PER_LINUX32, "sparc32bash", "sparc"},
    {PER_LINUX32, "sparc32", "sparc"},
    {PER_LINUX, "sparc64", "sparc64"},
#endif
#if defined(__x86_64__) || defined(__i386__) || defined(__ia64__)
    {PER_LINUX32, "i386", "i386"},
    {PER_LINUX32, "i486", "i386"},
    {PER_LINUX32, "i586", "i386"},
    {PER_LINUX32, "i686", "i386"},
    {PER_LINUX32, "athlon", "i386"},
#endif
#if defined(__x86_64__) || defined(__i386__)
    {PER_LINUX, "x86_64", "x86_64"},
#endif
    {-1, NULL, NULL}, /* this one is filled during init */
    {-1, NULL, NULL},
};

static void init_domains(void) {
    static int inited = 0;
    if (inited) {
        return;
    }
    inited = 1;

    static struct utsname un;
    uname(&un);

    size_t i;
    for (i = 0; domains[i].pval >= 0; ++i) {
        if (!strcmp(un.machine, domains[i].tarch)) {
            /* found our own arch */
            break;
        }
    }
    if (domains[i].pval < 0) {
        /* arch unknown at compile time, make up a value */
        int ws = 0;
        FILE *f = fopen("/sys/kernel/address_bits", "rb");
        if (f) {
            int v1 = fgetc(f);
            int v2 = fgetc(f);
            if ((v1 == '3') && (v2 == '2')) {
                ws = 32;
            } else if ((v1 == '6') && (v2 == '4')) {
                ws = 64;
            }
            fclose(f);
        }
        if (!ws) {
            /* fall back to compile-time value */
            ws = (sizeof(void *) * CHAR_BIT);
        }
        domains[i].pval = (ws == 32) ? PER_LINUX32 : PER_LINUX;
        domains[i].tarch = un.machine;
        domains[i].rarch = un.machine;
    }
}

static void list_domains(void) {
    init_domains();

    for (size_t i = 0; domains[i].tarch; ++i) {
        printf("%s\n", domains[i].tarch);
    }
}

static struct domain *get_domain(char const *pers) {
    init_domains();

    for (size_t i = 0; domains[i].tarch; ++i) {
        if (!strcmp(domains[i].tarch, pers)) {
            return &domains[i];
        }
    }

    return NULL;
}


#define OPT_ENABLE(flag) \
    options |= flag; \
    if (verbose) { \
        printf("Switching on " #flag ".\n"); \
    }

int main(int argc, char **argv) {
    int verbose = 0;
    int help = 0;
    int version = 0;
    int wrapped = 0;
    unsigned long pers = 0;
    unsigned long options = 0;

    char const *arch = strrchr(__progname, '/');
    if (!arch++) {
        arch = __progname;
    }

    if (!strcmp(arch, "setarch")) {
        if ((argc > 1) && (*argv[1] != '-')) {
            /* personality as first arg, treat it as zeroth arg */
            arch = argv[1];
            argv[1] = argv[0];
            ++argv;
            --argc;
        } else {
            arch = NULL;
        }
    } else {
        wrapped = 1;
    }

    for (;;) {
        int opt_idx = 0;
        int c = getopt_long(argc, argv, "+RFZLXBIST3vhV", gnuopts, &opt_idx);
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

            case 'R':
                OPT_ENABLE(ADDR_NO_RANDOMIZE);
                break;
            case 'F':
                OPT_ENABLE(FDPIC_FUNCPTRS);
                break;
            case 'Z':
                OPT_ENABLE(MMAP_PAGE_ZERO);
                break;
            case 'L':
                OPT_ENABLE(ADDR_COMPAT_LAYOUT);
                break;
            case 'X':
                OPT_ENABLE(READ_IMPLIES_EXEC);
                break;
            case 'B':
                OPT_ENABLE(ADDR_LIMIT_32BIT);
                break;
            case 'I':
                OPT_ENABLE(SHORT_INODE);
                break;
            case 'S':
                OPT_ENABLE(WHOLE_SECONDS);
                break;
            case 'T':
                OPT_ENABLE(STICKY_TIMEOUTS);
                break;
            case '3':
                OPT_ENABLE(ADDR_LIMIT_3GB);
                break;
            case LONGOPT_4GB:
                /* ignore */
                break;
            case LONGOPT_UNAME26:
                OPT_ENABLE(UNAME26);
                break;

            case LONGOPT_LIST:
                if (!wrapped) {
                    list_domains();
                    return 0;
                }
                fprintf(stderr, "%s: invalid option '--list'\n", __progname);
                goto errhelp;

            default:
                fprintf(stderr, "%s: invalid option -- '%c'\n", __progname, c);
            errhelp:
                fprintf(
                    stderr, "Try '%s --help' for more information.\n",
                    __progname
                );
                return 1;
        }
    }

    if (help) {
        if (wrapped) {
            printf("Usage: %s [OPTION]... [PROGRAM [OPTION]...]\n", __progname);
        } else {
            printf(
                "Usage: %s [ARCH] [OPTION]... [PROGRAM [OPTION]...]\n",
                 __progname
             );
        }
        printf(
"\n"
"Change the reported architecture and personality flags.\n"
"\n"
"      -B, --32bit               turn on ADDR_LIMIT_32BIT\n"
"      -F, --fdpic-funcptrs      make function pointers point to descriptors\n"
"      -I, --short-inode         turn on SHORT_INODE\n"
"      -L, --addr-compat-layout  change the way virtual memory is allocated\n"
"      -R, --addr-no-randomize   disable randomization of virtual address space\n"
"      -S, --whole-seconds       turn on WHOLE_SECONDS\n"
"      -T, --sticky-timeouts     turn on STICKY_TIMEOUTS\n"
"      -X, --read-implies-exec   turn on READ_IMPLIES_EXEC\n"
"      -Z, --mmap-page-zero      turn on MMAP_PAGE_ZERO\n"
"      -3, --3gb                 limit the used address space to 3GB\n"
"          --4gb                 ignored (for compatibility)\n"
"          --uname-2.6           pretend we are 2.6 kernel\n"
"      -h, --help                display this help and exit\n"
"      -V, --version             output version information and exit\n"
        );

        if (!wrapped) {
            printf(
"          --list                list settable architectures and exit\n"
            );
        }

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

    if (!arch && !options) {
        errx(1, "no architecture argument or personality flags specified");
    }

    argc -= optind;
    argv += optind;

    struct domain *tgt;

    if (arch) {
        tgt = get_domain(arch);
        if (!tgt) {
            errx(1, "%s: unrecognized architecture", arch);
        }
        pers = tgt->pval;
    }

    if (personality(pers | options) < 0) {
        /* we might need to do this twice as on some kernels the syscall
         * cannot return an error, but rather returns the previous
         * personality value
         */
        if (personality(pers | options) < 0) {
            err(1, "failed to set personality to %s", arch);
        }
    }

    if (arch && tgt->rarch) {
        struct utsname un;
        uname(&un);
        if (strcmp(un.machine, tgt->rarch)) {
            /* special case for i386 and arm */
            if (!strcmp(tgt->rarch, "i386") || !strcmp(tgt->rarch, "arm")) {
                for (size_t i = 0; domains[i].tarch; ++i) {
                    if (!domains[i].rarch) {
                        continue;
                    }
                    if (strcmp(domains[i].rarch, tgt->rarch)) {
                        continue;
                    }
                    if (!strcmp(domains[i].tarch, un.machine)) {
                        goto really_ok;
                    }
                }
            }
            errx(1, "could not set architecture to %s", arch);
        }
    }

really_ok:
    if (argc) {
        if (verbose) {
            printf("Execute command '%s'.\n", argv[0]);
            fflush(NULL);
        }
        execvp(argv[0], argv);
        err(1, "execvp: failed to execute '%s'", argv[0]);
        return 1;
    }

    /* we want a login shell */
    char sarg[sizeof(_PATH_BSHELL) + 1];
    memset(sarg, '-', sizeof(sarg));
    memcpy(&sarg[1], _PATH_BSHELL, sizeof(_PATH_BSHELL));

    char *sargp = strrchr(sarg, '/');
    if (sargp) {
        *sargp = '-';
    } else {
        sargp = sarg;
    }

    execl(_PATH_BSHELL, sargp, NULL);
    err(1, "execl");
    return 1;
}
