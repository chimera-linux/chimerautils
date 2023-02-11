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

#ifndef SYS_CAPSICUM_H
#define SYS_CAPSICUM_H

#define CAP_READ 0
#define CAP_WRITE 1
#define CAP_SEEK 2
#define CAP_FSTAT 3
#define CAP_FSYNC 4
#define CAP_FCNTL 5
#define CAP_FSTATFS 6
#define CAP_FTRUNCATE 7
#define CAP_IOCTL 8
#define CAP_MMAP_R 9
#define CAP_EVENT 10
#define CAP_LOOKUP 11
#define CAP_PWRITE 12

typedef struct cap_rights cap_rights_t;

struct cap_rights {
    int pad;
};

static inline cap_rights_t *cap_rights_init(cap_rights_t *rights, ...) {
    return rights;
}

static inline int caph_rights_limit(int fd, const cap_rights_t *rights) {
    (void)rights;
    (void)fd;
    return 0;
}

static inline cap_rights_t *cap_rights_set(cap_rights_t *rights, ...) {
    return rights;
}

static inline cap_rights_t *cap_rights_clear(cap_rights_t *rights, ...) {
    return rights;
}

static inline int cap_rights_is_set(cap_rights_t *rights, ...) {
    (void)rights;
    return 1;
}

#endif
