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
#include <getopt.h>
#include <unistd.h>
#include <err.h>
#include <sys/reboot.h>
#include <sys/syscall.h>
#include <linux/reboot.h>

extern char const *__progname;

static struct option gnuopts[] = {
    {"help",               no_argument, NULL, 'h'},
    {"version",            no_argument, NULL, 'V'},
    {0, 0, 0, 0}
};

int main(int argc, char **argv) {
    int help = 0;
    int version = 0;
    int rbmode = !strcmp(__progname, "reboot-mode");

    for (;;) {
        int opt_idx = 0;
        int c = getopt_long(argc, argv, "hV", gnuopts, &opt_idx);
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
        if (rbmode) {
            printf(
"Usage: %s MODE\n\n"
"Reboot the device to the MODE specified (e.g. recovery, bootloader).\n",
                __progname
            );
        } else {
            printf(
"Usage: %s hard|soft\n\n"
"Set the function of the Ctrl-Alt-Del combination.\n",
                __progname
            );
        }
        printf(
"\n"
"      -h, --help                display this help and exit\n"
"      -V, --version             output version information and exit\n"
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

    if (argc < 2) {
        if (rbmode) {
            errx(1, "argument is needed");
        }
        char const *ppath = "/proc/sys/kernel/ctrl-alt-del";
        FILE *f = fopen(ppath, "r");
        if (f) {
            char buf[32] = {0};
            if (fgets(buf, sizeof(buf), f)) {
                if (
                    ((buf[0] == '0') || (buf[0] == '1')) &&
                    ((buf[1] == '\0') || (buf[1] == '\n'))
                ) {
                    if (buf[0] == '0') {
                        printf("soft\n");
                    } else {
                        printf("hard\n");
                    }
                    fclose(f);
                    return 0;
                }
                printf("implicit hard\n");
                warn("unexpected value in %s: %s", ppath, buf);
                fclose(f);
                return 1;
            }
            fclose(f);
        }
        err(1, "cannot read %s", ppath);
    }

    if (geteuid() != 0) {
        errx(1, "you must be root");
    }

    unsigned int cmd;
    void *arg = NULL;

    if (rbmode) {
        cmd = LINUX_REBOOT_CMD_RESTART2;
        arg = argv[1];
        /* this actually reboots instantly, so make sure to sync first */
        sync();
    } else if (!strcmp(argv[1], "hard")) {
        cmd = LINUX_REBOOT_CMD_CAD_ON;
    } else if (!strcmp(argv[1], "soft")) {
        cmd = LINUX_REBOOT_CMD_CAD_OFF;
    } else {
        errx(1, "unknown argument: %s", argv[1]);
    }

    int ret = syscall(
        SYS_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, cmd, arg
    );
    if (ret < 0) {
        err(1, "reboot");
    }
    return 0;
}
