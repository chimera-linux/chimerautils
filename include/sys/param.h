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

#ifndef SYS_PARAM_H
#define SYS_PARAM_H

#include_next <sys/param.h>

/* max raw I/O transfer size */
/*
 * XXX: this is _probably_ going to be 1M on the system if it were
 * running FreeBSD.  What is the corresponding Linux parameter here
 * and the sanctioned way to retrieve it?
 */
#ifndef MAXPHYS
#define MAXPHYS (1024 * 1024)
#endif

/*
 * File system parameters and macros.
 *
 * MAXBSIZE - Filesystems are made out of blocks of at most MAXBSIZE
 *            bytes per block.  MAXBSIZE may be made larger without
 *            effecting any existing filesystems as long as it does
 *            not exceed MAXPHYS, and may be made smaller at the
 *            risk of not being able to use filesystems which
 *            require a block size exceeding MAXBSIZE.
 */
#ifndef MAXBSIZE
#define MAXBSIZE 65536 /* must be power of 2 */
#endif

#define roundup2(x, y)  (((x)+((y)-1))&(~((y)-1))) /* if y is powers of two */
#define nitems(x) (sizeof((x)) / sizeof((x)[0]))

#endif
