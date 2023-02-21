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
#include <string.h>
#include <err.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/statfs.h>
#include <linux/magic.h>

extern char const *__progname;

static void child(int fd) {
    DIR *dir = fdopendir(fd);
    if (!dir) {
        warn("fdopendir");
        close(fd);
        return;
    }

    int dfd = dirfd(dir);

    struct stat st;
    if (fstat(dfd, &st) < 0) {
        warn("fstat");
        goto done;
    }

    for (;;) {
        errno = 0;
        int isdir = 0;
        struct dirent *d = readdir(dir);
        if (!d) {
            if (errno) {
                warn("readdir");
                goto done;
            }
            /* done */
            break;
        }
        if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, "..")) {
            continue;
        }
        if ((d->d_type == DT_DIR) || (d->d_type == DT_UNKNOWN)) {
            struct stat cst;
            if (fstatat(dfd, d->d_name, &cst, AT_SYMLINK_NOFOLLOW)) {
                warn("fstatat(%s)", d->d_name);
            }
            /* crosses mounts */
            if (cst.st_dev != st.st_dev) {
                continue;
            }
            /* recurse */
            if (S_ISDIR(st.st_mode)) {
                int cfd = openat(dfd, d->d_name, O_RDONLY);
                if (cfd >= 0) {
                    child(cfd);
                }
                isdir = 1;
            }
        }
        if (unlinkat(dfd, d->d_name, isdir ? AT_REMOVEDIR : 0) < 0) {
            warn("unlinkat(%s)", d->d_name);
        }
    }

done:
    closedir(dir);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s new_root init [init_args]...\n", __progname);
    }

    char const *root = argv[1];
    char const *init = argv[2];
    char **iargs = &argv[2];

    if (!*root || !*init) {
        errx(1, "bad usage");
    }

    struct stat oroot;
    if (stat("/", &oroot) < 0) {
        err(1, "stat(old root)");
    }

    struct stat nroot;
    if (stat(root, &nroot) < 0) {
        err(1, "stat(new root)");
    }

    /* move pseudo-filesystems */
    char const *pfs[] = {"/dev", "/proc", "/run", "/sys", NULL};

    for (char const **fsp = pfs; *fsp; ++fsp) {
        char mnt[PATH_MAX];
        struct stat st;

        snprintf(mnt, sizeof(mnt), "%s%s", root, *fsp);

        if (!stat(*fsp, &st) && (st.st_dev == oroot.st_dev)) {
            /* same filesystem? */
            continue;
        }

        if ((stat(mnt, &st) < 0) || (st.st_dev != nroot.st_dev)) {
            /* mounted already? */
            umount2(*fsp, MNT_DETACH);
        }

        if (mount(*fsp, mnt, NULL, MS_MOVE, NULL) < 0) {
            warn("mount(%s -> %s), forcing unmount", *fsp, mnt);
            umount2(*fsp, MNT_FORCE);
        }
    }

    if (chdir(root) < 0) {
        err(1, "chdir(new root)");
    }

    int ofd = open("/", O_RDONLY);
    if (ofd < 0) {
        err(1, "open(old root)");
    }

    if (mount(root, "/", NULL, MS_MOVE, NULL) < 0) {
        close(ofd);
        err(1, "mount(new root, move)");
    }

    if (chroot(".") < 0) {
        err(1, "chroot");
    }

    if (chdir("/") < 0) {
        err(1, "chdir(/)");
    }

    pid_t fpid = fork();
    if (fpid < 0) {
        err(1, "fork");
    } else if (fpid) {
        /* parent */
        close(ofd);
        goto parent_init;
    }

    /* child */

    struct statfs sfs;
    if (!fstatfs(ofd, &sfs) && (
        (sfs.f_type == RAMFS_MAGIC) || (sfs.f_type == TMPFS_MAGIC)
    )) {
        /* clean up; dup to give DIR its own descriptor */
        int cfd = dup(ofd);
        if (cfd < 0) {
            warn("dup");
        } else {
            child(cfd);
        }
    } else {
        warnx("old rootfs is not an initramfs");
    }
    close(ofd);
    return 0;

parent_init:
    if (access(init, X_OK) < 0) {
        warn("cannot access %s", init);
    }

    execv(init, iargs);
    err(1, "execv");
    return 1;
}
