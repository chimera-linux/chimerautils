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
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>
#include <fcntl.h>
#include <getopt.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libutil.h>
#include <linux/falloc.h>

#ifndef SEEK_DATA
#define SEEK_DATA 3
#endif

#ifndef SEEK_HOLE
#define SEEK_HOLE 4
#endif

extern char *__progname;

static struct option gnuopts[] = {
    {"collapse-range", no_argument, NULL, 'c'},
    {"dig-holes",      no_argument, NULL, 'd'},
    {"insert-range",   no_argument, NULL, 'i'},
    {"length",   required_argument, NULL, 'l'},
    {"keep-size",      no_argument, NULL, 'n'},
    {"offset",   required_argument, NULL, 'o'},
    {"punch-hole",     no_argument, NULL, 'p'},
    {"posix",          no_argument, NULL, 'x'},
    {"zero-range",     no_argument, NULL, 'z'},
    {"verbose",        no_argument, NULL, 'v'},
    {"version",        no_argument, NULL, 'V'},
    {0, 0, 0, 0}
};

/* we could use bsd expand_number, but need to keep the -ib and
 * -b suffix distinction in order to stay compatible with util-linux
 */
static off_t expand_size(char const *argn) {
    char *endp = NULL;
    errno = 0;
    uintmax_t umax = strtoumax(optarg, &endp, 0);
    if (!endp || errno) {
        goto is_err;
    }
    unsigned int nmul = 0;
    switch (*endp++ | 32) {
        case 'e': nmul = 6; break;
        case 'p': nmul = 5; break;
        case 't': nmul = 4; break;
        case 'g': nmul = 3; break;
        case 'm': nmul = 2; break;
        case 'k': nmul = 1; break;
        case 'b':
        case '\0':
            goto check_offmax;
        default:
            goto is_err;
    }
    if (!*endp || !strcasecmp(endp, "ib")) {
        /* 1024 */
        unsigned int shift = nmul * 10;
        if (((umax << shift) >> shift) != umax) {
            goto is_err;
        }
        umax <<= shift;
        goto check_offmax;
    } else if (((*endp | 32) == 'b') && !endp[1]) {
        /* 1000 */
        while (nmul--) {
            uintmax_t numax = umax * 1000;
            if ((numax / 1000) != umax) {
                goto is_err;
            }
            umax = numax;
        }
        goto check_offmax;
    }
is_err:
    errx(1, "invalid %s argument '%s'", argn, optarg);
check_offmax:
    if (umax > (((uintmax_t)1 << ((CHAR_BIT * sizeof(off_t)) - 1)) - 1)) {
        goto is_err;
    }
    return (off_t)umax;
}

int main(int argc, char **argv) {
    int help = 0;
    int version = 0;
    int verbose = 0;
    int have_len = 0;
    int posix = 0;
    int flags = 0;
    int dig = 0;
    off_t length = 0;
    off_t offset = 0;
    char humbuf[9];

    for (;;) {
        int opt_idx = 0;
        int c = getopt_long(argc, argv, "cdhil:no:pxzvV", gnuopts, &opt_idx);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'c':
                flags |= FALLOC_FL_COLLAPSE_RANGE;
                break;

            case 'd':
                dig = 1;
                break;

            case 'i':
                flags |= FALLOC_FL_INSERT_RANGE;
                break;

            case 'l':
                length = expand_size("length");
                have_len = 1;
                break;

            case 'n':
                flags |= FALLOC_FL_KEEP_SIZE;
                break;

            case 'o':
                offset = expand_size("offset");
                break;

            case 'p':
                flags |= FALLOC_FL_PUNCH_HOLE;
                flags |= FALLOC_FL_KEEP_SIZE;
                break;

            case 'v':
                verbose = 1;
                break;

            case 'x':
                posix = 1;
                break;

            case 'z':
                flags |= FALLOC_FL_ZERO_RANGE;
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

    if (posix && dig) {
        errx(1, "-x is incompatible with -d");
    } else if (flags && (posix || dig)) {
        errx(
            1, "-%c is incompatible with either of -c, -i, -n, -p or -z",
            posix ? 'x' : 'd'
        );
    } else if (flags && (flags & (flags - 1))) {
        /* FALLOC_ flags are all bits, so we just check power of two */
        if (flags != (FALLOC_FL_KEEP_SIZE | FALLOC_FL_PUNCH_HOLE)) {
            errx(1, "-c, -i, -p and -z are mutually incompatible");
        }
    }

    if (help) {
        printf(
"Usage: %s [OPTION]... <file>\n"
"\n"
"Preallocate space to, or deallocate space from a file.\n"
"\n"
"      -c, --collapse-range  remove a range from the file\n"
"      -d, --dig-holes       detect zeroes and replace with holes\n"
"      -i, --insert-range    insert a hole at range, shifting existing data\n"
"      -l, --length NUM      length for range operations, in bytes\n"
"      -n, --keep-size       maintain the apparent size of the file\n"
"      -o, --offset NUM      offset for range operations, in bytes\n"
"      -p, --punch-hole      replace a range with a hole (implies -n)\n"
"      -z, --zero-range      zero and ensure allocation of a range\n"
"      -x, --posix           use posix_fallocate(3) instead of fallocate(2)\n"
"      -v, --verbose         verbose mode\n"
"      -h, --help            display this help and exit\n"
"      -V, --version         output version information and exit\n",
            __progname
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

    if (optind == argc) {
        errx(1, "no filename given");
    } else if ((optind + 1) != argc) {
        errx(1, "invalid number of arguments");
    }

    char const *fname = argv[optind];

    int fd = open(fname, O_RDWR | ((flags || dig) ? 0 : O_CREAT), DEFFILEMODE);
    if (fd < 0) {
        err(1, "open failed for %s", fname);
    }

    if (!dig) {
        int errv;
        if (!have_len) {
            errx(1, "length not specified");
        } else if (!length) {
            errx(1, "length must be non-zero");
        }
        errno = 0;
        if (posix) {
            errv = posix_fallocate(fd, offset, length);
        } else {
            errv = fallocate(fd, flags, offset, length);
        }
        if (errv < 0) {
            err(1, "fallocate");
        }
        if (verbose) {
            humanize_number(
                humbuf, sizeof(humbuf), length, "B", HN_AUTOSCALE,
                HN_DECIMAL | HN_IEC_PREFIXES
            );
            char const *word = "allocated";
            if (flags & FALLOC_FL_COLLAPSE_RANGE) {
                word = "removed";
            } else if (flags & FALLOC_FL_INSERT_RANGE) {
                word = "inserted";
            } else if (flags & FALLOC_FL_PUNCH_HOLE) {
                word = "hole created";
            } else if (flags & FALLOC_FL_ZERO_RANGE) {
                word = "zeroed";
            }
            printf("%s: %s (%ju bytes) %s.\n", fname, humbuf, length, word);
        }
        goto do_close;
    }

    /* dig holes */
    struct stat st;

    if (fstat(fd, &st) < 0) {
        err(1, "stat failed for %s", fname);
    }

    size_t bsz = st.st_blksize;

    off_t fend = length;
    if (fend) {
        fend += offset;
    }

    if (lseek(fd, offset, SEEK_SET) < 0) {
        err(1, "lseek failed for %s", fname);
    }

    unsigned char *buf = malloc(bsz);
    if (!buf) {
        err(1, "malloc");
    }

    /* 1M on 4K pages; the frequency for POSIX_FADV_DONTNEED
     *
     * posix_fadvise optimization taken from util-linux
     */
    size_t dontneed = getpagesize() * 256;
    off_t cbeg = offset;

    off_t holen = 0, holest = 0;
    uintmax_t total = 0;

    while (!fend || (offset < fend)) {
        /* locate data */
        off_t off = lseek(fd, offset, SEEK_DATA);
        /* beyond or error, or end capped */
        if ((off < 0) || (fend && (off >= fend))) {
            break;
        }
        /* locate hole */
        off_t hoff = lseek(fd, off, SEEK_HOLE);
        /* like above */
        if ((hoff < 0) || (fend && (off >= fend))) {
            hoff = fend;
        }
        posix_fadvise(fd, off, hoff, POSIX_FADV_SEQUENTIAL);
        /* dig holes */
        while (off < hoff) {
            ssize_t rsz = pread(fd, buf, bsz, off);
            if (rsz < 0) {
                err(1, "pread");
            } else if (!rsz) {
                break;
            }
            /* we read rsz bytes */
            if (hoff && ((off + rsz) > hoff)) {
                /* cap the read size so it does not go beyond hole */
                rsz = (hoff - off);
            }
            /* try to find nonzero byte in rsz bytes */
            int found = 0;
            {
                size_t left = rsz;
                uintptr_t *wbuf = (uintptr_t *)buf;
                /* first try to find nonzero word */
                for (; (left > sizeof(uintptr_t)) && !*wbuf; ++wbuf) {
                    left -= sizeof(uintptr_t);
                }
                /* now find nonzero byte */
                unsigned char *bbuf = (unsigned char *)wbuf;
                for (; left--; ++bbuf) {
                    if (*bbuf++) {
                        found = 1;
                        break;
                    }
                }
            }
            if (!found) {
                /* it's all zeroes */
                if (!holen) {
                    /* mark beginning of hole */
                    holest = off;
                }
                /* grow hole size */
                holen += rsz;
            } else if (holen) {
                /* encountered a block with non-zero bytes, and we were
                 * previously tracking a hole; punch it and reset the count
                 */
                if (fallocate(
                    fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE,
                    holest, holen
                ) < 0) {
                    err(1, "fallocate");
                }
                total += holen;
                holest = 0;
                holen = 0;
            }
            /* discard */
            if ((size_t)off > (cbeg + dontneed)) {
                /* how many dontneed blocks are in the range */
                size_t n = (off - cbeg) / dontneed;
                posix_fadvise(fd, cbeg, n * dontneed, POSIX_FADV_DONTNEED);
                cbeg += (n * dontneed);
            }
            /* either way, move on by rsz */
            off += rsz;
        }
        if (holen) {
            /* punch whatever we had left */
            if (fallocate(
                fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, holest,
                holen + ((off >= hoff) ? bsz : 0)
            ) < 0) {
                err(1, "fallocate");
            }
            total += holen;
        }
        offset = off;
    }

    free(buf);

    if (verbose) {
        humanize_number(
            humbuf, sizeof(humbuf), total, "B", HN_AUTOSCALE,
            HN_DECIMAL | HN_IEC_PREFIXES
        );
        printf(
            "%s: %s (%ju bytes) converted to sparse holes.\n",
            fname, humbuf, total
        );
    }

do_close:
    if (fsync(fd) < 0) {
        err(1, "fsync");
    }
    if (close(fd) < 0) {
        err(1, "close");
    }

    return 0;
}
