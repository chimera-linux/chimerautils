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
#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <linux/blkpg.h>

extern char const *__progname;
/* 512-byte sectors */
unsigned long long maxsect = ULLONG_MAX >> 9;

static unsigned long long get_part_start(int fd, int partn, char **argv) {
    struct stat st;
    if (fstat(fd, &st) < 0) {
        err(1, "fstat");
    }
    if (!S_ISBLK(st.st_mode)) {
        goto generror;
    }

    dev_t devn = st.st_rdev;

    int maj = major(devn);
    /* partition minor is the whole disk minor + part number */
    int min = minor(devn) + partn;

    char buf[PATH_MAX];
    /* this is in 512-byte sectors too */
    snprintf(buf, sizeof(buf), "/sys/dev/block/%d:%d/start", maj, min);

    FILE *f = fopen(buf, "rb");
    if (!f) {
        err(1, "fopen");
    }

    unsigned long long ret;
    if ((fscanf(f, "%llu", &ret) != 1) || (ret > maxsect)) {
        fclose(f);
        goto generror;
    }

    fclose(f);
    return ret;

generror:
    errx(1, "%s: could not get start for part num %s", argv[1], argv[2]);
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(
            stderr, "usage: %s disk_device part_number length\n", __progname
        );
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        err(1, "open");
    }

    char *errp = NULL;
    unsigned long pnum = strtoul(argv[2], &errp, 10);
    if (!errp || *errp || (pnum > INT_MAX)) {
        errx(1, "invalid partition number");
    }

    errp = NULL;
    unsigned long long length = strtoull(argv[3], &errp, 10);
    if (!errp || *errp || (length > maxsect)) {
        errx(1, "invalid length argument");
    }

    unsigned long long start = get_part_start(fd, (int)pnum, argv);

    struct blkpg_partition part = {
        .start = start << 9,
        .length = length << 9,
        .pno = (int)pnum,
        .devname[0] = '\0',
        .volname[0] = '\0',
    };
    struct blkpg_ioctl_arg arg = {
        .op = BLKPG_RESIZE_PARTITION,
        .flags = 0,
        .datalen = sizeof(part),
        .data = &part,
    };

    if (ioctl(fd, BLKPG, &arg) < 0) {
        err(1, "ioctl");
    }
    return 0;
}
