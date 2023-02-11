/*-
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

#ifndef CASPER_CAP_FILEARGS_H
#define CASPER_CAP_FILEARGS_H

#include <stdio.h>
#include <fcntl.h>
#include <libcasper.h>
#include <sys/capsicum.h>
#include <sys/stat.h>

#define FA_OPEN 0
#define FA_REALPATH 1

typedef struct fileargs_t fileargs_t;

static fileargs_t *_fa = (void *)0xDEADBEEF;

static inline fileargs_t *fileargs_init(
    int argc, char *argv[], int flags,
    mode_t mode, cap_rights_t *rightsp, int operations
) {
    (void)argc;
    (void)argv;
    (void)flags;
    (void)mode;
    (void)rightsp;
    (void)operations;
    return _fa;
}

static inline fileargs_t *fileargs_cinit(
    cap_channel_t *cas, int argc, char *argv[], int flags, mode_t mode,
    cap_rights_t *rightsp, int operations
) {
    (void)cas;
    return fileargs_init(argc, argv, flags, mode, rightsp, operations);
}

static inline int fileargs_open(fileargs_t *fa, const char *path) {
    (void)fa;
    return open(path, O_RDONLY);
}

static inline FILE *fileargs_fopen(fileargs_t *fa, const char *path, const char *mode) {
    (void)fa;
    return fopen(path, mode);
}

static inline void fileargs_free(fileargs_t *fa) {
    (void)fa;
}

#endif
