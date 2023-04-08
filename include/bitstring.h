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

#ifndef BITSTRING_H
#define BITSTRING_H

typedef	unsigned long bitstr_t;

#define	_BITSTR_BITS (sizeof(bitstr_t) * 8)

#define _bit_roundup2(x, y) (((x)+((y)-1))&(~((y)-1)))
#define _bit_idx(bit) ((bit) / _BITSTR_BITS)
#define _bit_offset(bit) ((bit) % _BITSTR_BITS)
#define _bit_mask(bit) (1UL << _bit_offset(bit))

#define	bitstr_size(_nbits) (_bit_roundup2(_nbits, _BITSTR_BITS) / 8)
#define	bit_decl(name, nbits) ((name)[bitstr_size(nbits) / sizeof(bitstr_t)])

static inline int bit_test(const bitstr_t *bs, int bit) {
    return ((bs[_bit_idx(bit)] & _bit_mask(bit)) != 0);
}

static inline void bit_set(bitstr_t *bs, int bit) {
    bs[_bit_idx(bit)] |= _bit_mask(bit);
}

#endif

