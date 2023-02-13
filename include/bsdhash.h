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

#ifndef BSDHASH_H
#define BSDHASH_H

#include <openssl/evp.h>
#include <stdio.h>
#include <stddef.h>
#include <err.h>

typedef EVP_MD_CTX *MD5_CTX;
typedef EVP_MD_CTX *RIPEMD160_CTX;
typedef EVP_MD_CTX *SHA1_CTX;
typedef EVP_MD_CTX *SHA256_CTX;
typedef EVP_MD_CTX *SHA512_CTX;

static inline void HashInit(EVP_MD_CTX **ctx, const EVP_MD *type) {
    *ctx = EVP_MD_CTX_new();
    if (!*ctx)
        errx(1, "could not init context");
    if (!EVP_DigestInit_ex(*ctx, type, NULL))
        errx(1, "could not init digest");
}

static inline void HashUpdate(EVP_MD_CTX **ctx, const void *data, unsigned int len) {
    if (!EVP_DigestUpdate(*ctx, data, len))
        errx(1, "could not update digest");
}

static inline void HashFinal(unsigned char *digest, EVP_MD_CTX **ctx) {
    if (!EVP_DigestFinal(*ctx, digest, NULL))
        errx(1, "could not finalize digest");
}

static inline char *HashEnd(EVP_MD_CTX **ctx, char *buf) {
    unsigned char digbuf[EVP_MAX_MD_SIZE + 1];
    unsigned int mdlen = 0;

    if (!EVP_DigestFinal(*ctx, digbuf, &mdlen))
        errx(1, "failed to finalize digest");

    if (!buf) {
        buf = malloc(mdlen * 2 + 1);
        if (!buf) errx(1, "unable to allocate buffer");
    }

    for (unsigned int i = 0; i < mdlen; ++i)
        sprintf(buf + (i * 2), "%02x", digbuf[i]);

    return buf;
}

static inline char *HashFile(const char *name, char *ibuf, const EVP_MD *type) {
    EVP_MD_CTX *ctx;
    char *buf;
    FILE *f;

    f = fopen(name, "rb");
    if (!f) errx(1, "unable to open file %s", name);

    buf = ibuf;
    if (!buf) buf = malloc(16 * 1024);
    if (!buf) {
        fclose(f);
        errx(1, "unable to allocate buffer");
    }

    HashInit(&ctx, type);
    for (;;) {
        size_t n = fread(buf, 1, 16 * 1024, f);
        HashUpdate(&ctx, buf, n);
        if (n != (16 * 1024)) {
            if (feof(f)) break;
            if (ferror(f)) {
                if (!buf) free(buf);
                fclose(f);
                errx(1, "unable to read file %s", name);
            }
        }
    }

    fclose(f);
    return HashEnd(&ctx, NULL);
}

#define MD5_DIGEST_LENGTH 16

#define MD5Init(ctx) HashInit(ctx, EVP_md5())
#define MD5Update HashUpdate
#define MD5Final HashFinal
#define MD5End HashEnd
#define MD5File(name, buf) HashFile(name, buf, EVP_md5())

#define RIPEMD160_Init(ctx) HashInit(ctx, EVP_ripemd160())
#define RIPEMD160_Update HashUpdate
#define RIPEMD160_Final HashFinal
#define RIPEMD160_End HashEnd
#define RIPEMD160_File(name, buf) HashFile(name, buf, EVP_ripemd160())

#define SHA1_Init(ctx) HashInit(ctx, EVP_sha1())
#define SHA1_Update HashUpdate
#define SHA1_Final HashFinal
#define SHA1_End HashEnd
#define SHA1_File(name, buf) HashFile(name, buf, EVP_sha1())

#define SHA256_Init(ctx) HashInit(ctx, EVP_sha256())
#define SHA256_Update HashUpdate
#define SHA256_Final HashFinal
#define SHA256_End HashEnd
#define SHA256_File(name, buf) HashFile(name, buf, EVP_sha256())

#define SHA512_Init(ctx) HashInit(ctx, EVP_sha512())
#define SHA512_Update HashUpdate
#define SHA512_Final HashFinal
#define SHA512_End HashEnd
#define SHA512_File(name, buf) HashFile(name, buf, EVP_sha512())

#endif

