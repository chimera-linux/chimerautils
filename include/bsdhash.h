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
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>
#include <err.h>

typedef EVP_MD_CTX *MD5_CTX;
typedef EVP_MD_CTX *SHA1_CTX;
typedef EVP_MD_CTX *SHA224_CTX;
typedef EVP_MD_CTX *SHA256_CTX;
typedef EVP_MD_CTX *SHA384_CTX;
typedef EVP_MD_CTX *SHA512_CTX;
typedef EVP_MD_CTX *SHA512_224_CTX;
typedef EVP_MD_CTX *SHA512_256_CTX;

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

static inline char *HashFile(const char *name, char *buf, const EVP_MD *type) {
    EVP_MD_CTX *ctx;
    char *fdbuf;

    int fd = open(name, O_RDONLY);
    if (fd < 0) err(1, "unable to open file %s", name);

    fdbuf = malloc(16 * 1024);
    if (!fdbuf) {
        err(1, "out of memory");
    }

    HashInit(&ctx, type);
    for (;;) {
        ssize_t n = read(fd, fdbuf, 16 * 1024);
        if (n < 0) {
            err(1, "unable to read from file %s", name);
        }
        if (n) {
            HashUpdate(&ctx, fdbuf, n);
        }
        if (n != (16 * 1024)) {
            break;
        }
    }

    close(fd);

    return HashEnd(&ctx, buf);
}

static inline char *HashData(const void *data, unsigned int len, char *buf, const EVP_MD *type) {
    EVP_MD_CTX *ctx;
    HashInit(&ctx, type);
    HashUpdate(&ctx, data, len);
    return HashEnd(&ctx, buf);
}

#define MD5_DIGEST_LENGTH 16

#define BSD_HASH_FUNCS(dn, dnl) \
static inline void dn##_Init(dn##_CTX *ctx) { \
    HashInit(ctx, EVP_##dnl()); \
} \
static inline void dn##_Update(dn##_CTX *ctx, const void *data, unsigned int len) { \
    HashUpdate(ctx, data, len); \
} \
static inline void dn##_Final(unsigned char *digest, dn##_CTX *ctx) { \
    HashFinal(digest, ctx); \
} \
static inline char *dn##_End(dn##_CTX *ctx, char *buf) { \
    return HashEnd(ctx, buf); \
} \
static inline char *dn##_File(const char *name, char *buf) { \
    return HashFile(name, buf, EVP_##dnl()); \
} \
static inline char *dn##_Data(const void *data, unsigned int len, char *buf) { \
    return HashData(data, len, buf, EVP_##dnl()); \
}

BSD_HASH_FUNCS(MD5, md5)
BSD_HASH_FUNCS(SHA1, sha1)
BSD_HASH_FUNCS(SHA224, sha224)
BSD_HASH_FUNCS(SHA256, sha256)
BSD_HASH_FUNCS(SHA384, sha384)
BSD_HASH_FUNCS(SHA512, sha512)
BSD_HASH_FUNCS(SHA512_224, sha512_224)
BSD_HASH_FUNCS(SHA512_256, sha512_256)

#define MD5Init MD5_Init
#define MD5Update MD5_Update
#define MD5Final MD5_Final
#define MD5End MD5_End
#define MD5File MD5_File
#define MD5Data MD5_Data

#endif

