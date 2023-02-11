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

#ifndef CAPSICUM_HELPERS_H
#define CAPSICUM_HELPERS_H

#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/capsicum.h>

#define	CAPH_IGNORE_EBADF	0x0001
#define	CAPH_READ		0x0002
#define	CAPH_WRITE		0x0004
#define	CAPH_LOOKUP		0x0008

#define CAP_FCNTL_GETFL F_GETFL
#define CAP_FCNTL_SETFL F_SETFL

static inline int caph_limit_stream(int fd, int flags) {
    (void)fd;
    (void)flags;
    return 0;
}

static inline int caph_limit_stdio(void) {
    return 0;
}

static inline int caph_limit_stdin(void) {
    return 0;
}

static inline int caph_limit_stdout(void) {
    return 0;
}

static inline int caph_limit_stderr(void) {
    return 0;
}

static inline int caph_enter(void) {
    return 0;
}

static inline void caph_cache_catpages(void) {
}

static inline void caph_cache_tzdata(void) {
}

static inline int caph_enter_casper(void) {
    return 0;
}

static inline int caph_ioctls_limit(int fd, const unsigned long *cmds, size_t ncmds) {
    (void)fd;
    (void)cmds;
    (void)ncmds;
    return 0;
}

static inline int caph_fcntls_limit(int fd, uint32_t fcntlrights) {
    (void)fd;
    (void)fcntlrights;
    return 0;
}

#endif
