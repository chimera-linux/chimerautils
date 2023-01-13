/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Daniel Kolesa
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>

#include "config.h"

/* the size used for buffers
 *
 * the input buffer must be a multiple of 3, 4 and 5; that allows us to
 * guarantee that any partial read of data from input file will result in
 * a buffer that can be encoded in its entirety without needing more data,
 * which simplifies handling of padding characters
 *
 * this does not apply for decoding, where the input data may contain newlines
 * which must be skipped during the decoding, and we have no way to know how
 * many of these there will be, so we have handling of that via the overread
 * variable mechanism (which cannot trivially be used when encoding
 *
 * the output buffer is used in order to bypass stdio for small writes, and
 * instead only dump the whole thing once full
 */

#define IBUFSIZE (60 * 512)
#define OBUFSIZE 8192

/* available encodings */

enum mode {
    MODE_DEFAULT = 0,
    MODE_BASE32,
    MODE_BASE64,
};

enum encoding {
    ENCODING_UNKNOWN = 0,
    ENCODING_BASE64,
    ENCODING_BASE64URL,
    ENCODING_BASE32,
    ENCODING_BASE32HEX,
    ENCODING_BASE16,
    ENCODING_BASE2MSBF,
    ENCODING_BASE2LSBF,
    ENCODING_Z85,
};

static enum mode program_mode = MODE_DEFAULT;

/* alphabets for available encodings */

static const char b64_alpha[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const unsigned char b64_dtbl[] = {
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 97, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 62, 99,
    99, 99, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 99, 99,
    99, 98, 99, 99, 99,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 99, 99, 99, 99, 99, 99, 26, 27, 28, 29, 30, 31, 32, 33,
    34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99
};

static const char b64url_alpha[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

static const unsigned char b64url_dtbl[] = {
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 97, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    62, 99, 99, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 99, 99,
    99, 98, 99, 99, 99,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 99, 99, 99, 99, 63, 99, 26, 27, 28, 29, 30, 31, 32, 33,
    34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99
};

static const char b32_alpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

static const unsigned char b32_dtbl[] = {
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 97, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 26, 27, 28, 29, 30, 31, 99, 99, 99, 99,
    99, 99, 99, 99, 99,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99
};

static const char b32hex_alpha[] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";

static const unsigned char b32hex_dtbl[] = {
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 97, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 99, 99,
    99, 99, 99, 99, 99, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99
};

static const char b16_alpha[] = "0123456789ABCDEF";

static const unsigned char b16_dtbl[] = {
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 97, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 99, 99,
    99, 99, 99, 99, 99, 10, 11, 12, 13, 14, 15, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99
};

static const char z85_alpha[] =
    "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-:+=^!/*?&<>()[]{}@%$#";

static const unsigned char z85_dtbl[] = {
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 97, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 68, 99, 84, 83, 82, 72, 99, 75, 76, 70, 65, 99,
    63, 62, 69,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 64, 99,
    73, 66, 74, 71, 81, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
    46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, 77, 99, 78, 67, 99, 99, 10, 11, 12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 79, 99, 80, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99
};

/* current implementation variables */

static size_t (*base_basenc)(
    const unsigned char *buf, size_t n, char *obuf, size_t obufs,
    size_t *wrote, size_t *overread
) = NULL;

static const char *base_alpha = NULL;
static const unsigned char *base_dtbl = NULL;

static unsigned int dec_failed = 0;
static unsigned int dec_done = 0;
static unsigned long wrap = 76;

/* option handling */

extern char *__progname;

static int opt_decode, opt_ignore_garbage;

static struct option basencopts[] = {
    {"base64",         no_argument,       0, ENCODING_BASE64},
    {"base64url",      no_argument,       0, ENCODING_BASE64URL},
    {"base32",         no_argument,       0, ENCODING_BASE32},
    {"base32hex",      no_argument,       0, ENCODING_BASE32HEX},
    {"base16",         no_argument,       0, ENCODING_BASE16},
    {"base2msbf",      no_argument,       0, ENCODING_BASE2MSBF},
    {"base2lsbf",      no_argument,       0, ENCODING_BASE2LSBF},
    {"z85",            no_argument,       0, ENCODING_Z85},
    {"decode",         no_argument,       &opt_decode,         1},
    {"ignore-garbage", no_argument,       &opt_ignore_garbage, 1},
    {"wrap",           required_argument, NULL,              'w'},
    {"help",           no_argument,       NULL,              'h'},
    {"version",        no_argument,       NULL,              'v'},
    {0, 0, 0, 0}
};

static struct option baseopts[] = {
    {"decode",         no_argument,       &opt_decode,         1},
    {"ignore-garbage", no_argument,       &opt_ignore_garbage, 1},
    {"wrap",           required_argument, NULL,              'w'},
    {"help",           no_argument,       NULL,              'h'},
    {"version",        no_argument,       NULL,              'v'},
    {0, 0, 0, 0}
};

static void usage(FILE *stream) {
    fprintf(stream,
"Usage: %s [OPTION]... [FILE]\n"
"basenc encode or decode FILE, or standard input, to standard output.\n"
"\n"
"With no FILE, or when FILE is -, read standard input.\n"
"\n"
"Mandatory arguments to long options are mandatory for short options too.\n",
        __progname
    );
    if (program_mode == MODE_DEFAULT) {
        fprintf(stream,
"      --base64          same as 'base64' program (RFC4648 section 4)\n"
"      --base64url       file- and url-safe base64 (RFC4648 section 5)\n"
"      --base32          same as 'base32' program (RFC4648 section 6)\n"
"      --base32hex       extended hex alphabet base32 (RFC4648 section 7)\n"
"      --base16          hex encoding (RFC4648 section 8)\n"
"      --base2msbf       bit string with most significant bit (msb) first\n"
"      --base2lsbf       bit string with least significant bit (lsb) first\n"
        );
    }
    fprintf(stream,
"  -d, --decode          decode data\n"
"  -i, --ignore-garbage  when decoding, ignore non-alphabet characters\n"
"  -w, --wrap=COLS       wrap encoded lines after COLS character (default 76).\n"
"                          Use 0 to disable line wrapping\n"
"\n"
    );
    if (program_mode == MODE_DEFAULT) {
        fprintf(stream,
"      --z85             ascii85-like encoding (ZeroMQ spec:32/Z85);\n"
"                        when encoding, input length must be a multiple of 4;\n"
"                        when decoding, input length must be a multiple of 5\n"
        );
    }
    fprintf(stream,
"      --help     display this help and exit\n"
"      --version  output version information and exit\n"
"\n"
    );
    if (program_mode == MODE_DEFAULT) {
        fprintf(stream,
"When decoding, the input may contain newlines in addition to the bytes of\n"
"the formal alphabet.  Use --ignore-garbage to attempt to recover\n"
"from any other non-alphabet bytes in the encoded stream.\n"
        );
    } else {
        const char *encoding = "base64";
        if (program_mode == MODE_BASE32) {
            encoding = "base32";
        }
        fprintf(stream,
"The data are encoded as described for the %s alphabet in RFC 4648.\n"
"When decoding, the input may contain newlines in addition to the bytes of\n"
"the formal base32 alphabet.  Use --ignore-garbage to attempt to recover\n"
"from any other non-alphabet bytes in the encoded stream.\n",
            encoding
        );
    }
}

static void dump(char *buf, size_t n, size_t *wrapleft) {
    while (wrap && (n > *wrapleft)) {
        fwrite(buf, 1, *wrapleft, stdout);
        fputc('\n', stdout);
        buf += *wrapleft;
        n -= *wrapleft;
        *wrapleft = wrap;
    }
    fwrite(buf, 1, n, stdout);
    if (wrap) {
        *wrapleft -= n;
    }
}

static int do_basenc(FILE *fstream, char *buf, char *obuf, const char *fpath) {
    size_t taccum = 0;
    size_t wrapleft = wrap;
    size_t overread = 0;
    for (;;) {
        if (dec_done) {
            dec_failed = 1;
            break;
        }
        size_t n = fread(buf + overread, 1, IBUFSIZE - overread, fstream);
        size_t wrote;
        size_t left;
        if (n > 0) {
            n += overread;
        } else if (overread) {
            dec_failed = 1;
            break;
        }
        overread = 0;
        for (;;) {
            /* encode into our buffer; left == how much left in input */
            left = base_basenc(
                (const unsigned char *)buf, n, obuf + taccum,
                OBUFSIZE - taccum, &wrote, &overread
            );
            /* account for what we wrote */
            taccum += wrote;
            /* nothing left: encoded completely */
            if (!left) {
                break;
            }
            /* we haven't read enough into the buffer; try reading more */
            if (overread) {
                memmove(buf, buf + n - overread, overread);
                break;
            }
            /* otherwise our output buffer was not enough, dump it */
            dump(obuf, taccum, &wrapleft);
            obuf = buf + IBUFSIZE;
            taccum = 0;
            /* increment input buffer */
            buf += (n - left);
            n = left;
        }
        if (n != IBUFSIZE) {
            if (feof(fstream)) {
                break;
            }
            if (ferror(fstream)) {
                fprintf(stderr, "%s: ", __progname);
                perror(fpath);
                return 0;
            }
        }
    }
    if (overread) {
        dec_failed = 1;
    }
    /* anything further left in buffer: dump */
    if (taccum) {
        dump(buf + IBUFSIZE, taccum, &wrapleft);
    }
    return 1;
}

/* base64, base32, base16, z85 + variants */

static inline size_t base_dec(
    const unsigned char *buf, size_t n, char *obuf, size_t obufs,
    size_t *wrote, size_t *overread,
    const size_t inchars, const size_t outchars, const size_t base
) {
    *wrote = 0;
    while ((n > 0) && (buf[0] == '\n')) {
        ++buf;
        --n;
    }
    while (n >= inchars) {
        if (obufs < outchars) {
            return n;
        }
        uint64_t v = 0;
        size_t begn = n;
        for (size_t i = 0; i < inchars; ++i) {
            unsigned char cv = base_dtbl[buf[i]];
lbeg:
            switch (cv) {
                case 97:
maybe_garbage:
                    /* if not enough characters remain to make up the rest of
                     * the unit, it means the decoding has failed (bad input)
                     */
                    if ((n - 1) < (inchars - i)) {
                        *overread = begn;
                        return begn;
                    }
                    /* ignore newlines when decoding */
                    ++buf;
                    --n;
                    cv = base_dtbl[buf[i]];
                    goto lbeg;
                case 98:
                    for (size_t j = i; j < inchars; ++j) {
                        if (buf[j] != '=') {
                            dec_failed = 1;
                            return 0;
                        }
                    }
                    dec_done = 1;
                    *wrote -= (outchars - i + 1);
                    for (size_t j = 0; j < (outchars - i + 1); ++j) {
                        v *= base;
                    }
                    goto wbuf;
                case 99:
                    if (opt_ignore_garbage) {
                        goto maybe_garbage;
                    }
                    dec_failed = 1;
                    return 0;
                default:
                    break;
            }
            v = (v * base) + cv;
        }
wbuf:
        for (size_t i = 0; i < outchars; ++i) {
            obuf[i] = (v >> (outchars - i - 1) * 8) & 0xFF;
        }
        obuf += outchars;
        obufs -= outchars;
        *wrote += outchars;
        buf += inchars;
        n -= inchars;
        if (dec_done) {
            break;
        }
    }
    while ((n > 0) && (buf[0] == '\n')) {
        ++buf;
        --n;
    }
    if (n > 0) {
        *overread = n;
        return n;
    }
    return 0;
}

static inline size_t base64_decode(
    const unsigned char *buf, size_t n, char *obuf, size_t obufs,
    size_t *wrote, size_t *overread
) {
    return base_dec(buf, n, obuf, obufs, wrote, overread, 4, 3, 64);
}

static inline size_t base32_decode(
    const unsigned char *buf, size_t n, char *obuf, size_t obufs,
    size_t *wrote, size_t *overread
) {
    return base_dec(buf, n, obuf, obufs, wrote, overread, 8, 5, 32);
}

static inline size_t base16_decode(
    const unsigned char *buf, size_t n, char *obuf, size_t obufs,
    size_t *wrote, size_t *overread
) {
    return base_dec(buf, n, obuf, obufs, wrote, overread, 2, 1, 16);
}

static inline size_t z85_decode(
    const unsigned char *buf, size_t n, char *obuf, size_t obufs,
    size_t *wrote, size_t *overread
) {
    return base_dec(buf, n, obuf, obufs, wrote, overread, 5, 4, 85);
}

static inline size_t base2_decode(
    const unsigned char *buf, size_t n, char *obuf, size_t obufs,
    size_t *wrote, size_t *overread, const int lsbf
) {
    *wrote = 0;
    while ((n > 0) && (buf[0] == '\n')) {
        ++buf;
        --n;
    }
    uint8_t v = 0;
    size_t nr = 0;
    while (n > 0) {
        if (!obufs) {
            return n;
        }
        switch (*buf) {
            case '0':
            case '1':
                if (lsbf) {
                    v |= (*buf - 48) << nr;
                } else {
                    v |= (*buf - 48) << (7 - nr);
                }
                if (++nr == 8) {
                    *obuf++ = v;
                    *wrote += 1;
                    v = 0;
                    nr = 0;
                }
                break;
            case '\n':
                break;
            default:
                if (opt_ignore_garbage) {
                    break;
                }
                dec_failed = 1;
                return 0;
        }
        ++buf;
        --n;
    }
    if (nr > 0) {
        *overread = nr;
        return nr;
    }
    while ((n > 0) && (buf[0] == '\n')) {
        ++buf;
        --n;
    }
    return 0;
}

static size_t base2msbf_decode(
    const unsigned char *buf, size_t n, char *obuf, size_t obufs,
    size_t *wrote, size_t *overread
) {
    return base2_decode(buf, n, obuf, obufs, wrote, overread, 0);
}

static size_t base2lsbf_decode(
    const unsigned char *buf, size_t n, char *obuf, size_t obufs,
    size_t *wrote, size_t *overread
) {
    return base2_decode(buf, n, obuf, obufs, wrote, overread, 1);
}

static inline size_t base_enc(
    const unsigned char *buf, size_t n, char *obuf, size_t obufs, size_t *wrote,
    const size_t inchars, const size_t outchars, const size_t base
) {
    *wrote = 0;
    size_t nperchar = (inchars * 8) / outchars;
    /* precompute a divisor from constants */
    uint64_t outdiv = 1;
    for (size_t i = 1; i < outchars; ++i) {
        outdiv *= base;
    }
    /* main loop */
    while (n) {
        /* if not enough space in the buffer, leave it for next time */
        if (obufs < outchars) {
            return n;
        }
        /* number of bytes we're processing */
        size_t np = (n < inchars) ? n : inchars;
        /* compute the input number we're processing */
        uint64_t x = 0;
        for (size_t i = 0; i < np; ++i) {
            x |= ((uint64_t)buf[i] << ((inchars - i - 1) * 8));
        }
        uint64_t div = outdiv;
        /* how many characters we can actually encode */
        size_t rout = ((np * 8) + nperchar - 1) / nperchar;
        /* stuff we can encode */
        for (size_t i = 0; i < rout; ++i) {
            obuf[i] = base_alpha[(x / div) % base];
            div /= base;
        }
        /* padding */
        for (size_t i = rout; i < outchars; ++i) {
            obuf[i] = '=';
        }
        /* advance */
        obuf += outchars;
        obufs -= outchars;
        *wrote += outchars;
        buf += np;
        n -= np;
    }
    return 0;
}

static size_t base64_encode(
    const unsigned char *buf, size_t n, char *obuf, size_t obufs,
    size_t *wrote, size_t *overread
) {
    *overread = 0;
    return base_enc(buf, n, obuf, obufs, wrote, 3, 4, 64);
}

static size_t base32_encode(
    const unsigned char *buf, size_t n, char *obuf, size_t obufs,
    size_t *wrote, size_t *overread
) {
    *overread = 0;
    return base_enc(buf, n, obuf, obufs, wrote, 5, 8, 32);
}

static size_t base16_encode(
    const unsigned char *buf, size_t n, char *obuf, size_t obufs,
    size_t *wrote, size_t *overread
) {
    *overread = 0;
    return base_enc(buf, n, obuf, obufs, wrote, 1, 2, 16);
}

static size_t z85_encode(
    const unsigned char *buf, size_t n, char *obuf, size_t obufs,
    size_t *wrote, size_t *overread
) {
    if ((n % 4) != 0) {
        fprintf(
            stderr,
            "%s: invalid input (length must be a multiple of 4 characters)",
            __progname
        );
        return 0;
    }
    *overread = 0;
    return base_enc(buf, n, obuf, obufs, wrote, 4, 5, 85);
}

/* base2 */

static size_t base2_encode(
    const unsigned char *buf, size_t n, char *obuf, size_t obufs,
    size_t *wrote, size_t *overread, const int lsbf
) {
    *wrote = 0;
    *overread = 0;
    while (n > 0) {
        if (obufs < 8) {
            return n;
        }
        for (int i = 0; i < 8; ++i) {
            if (lsbf) {
                obuf[i] = ((buf[0] >> i) & 1) + 48;
            } else {
                obuf[7 - i] = ((buf[0] >> i) & 1) + 48;
            }
        }
        obuf += 8;
        *wrote += 8;
        ++buf;
        --n;
    }
    return 0;
}

static size_t base2lsbf_encode(
    const unsigned char *buf, size_t n, char *obuf, size_t obufs,
    size_t *wrote, size_t *overread
) {
    return base2_encode(buf, n, obuf, obufs, wrote, overread, 1);
}

static size_t base2msbf_encode(
    const unsigned char *buf, size_t n, char *obuf, size_t obufs,
    size_t *wrote, size_t *overread
) {
    return base2_encode(buf, n, obuf, obufs, wrote, overread, 0);
}

/* main */

int main(int argc, char **argv) {
    const char *fpath;
    int encoding = ENCODING_UNKNOWN;

    opterr = 0;

    if (!strcmp(__progname, "base32")) {
        program_mode = MODE_BASE32;
        encoding = ENCODING_BASE32;
    } else if (!strcmp(__progname, "base64")) {
        program_mode = MODE_BASE64;
        encoding = ENCODING_BASE64;
    }

    for (;;) {
        int opt_idx;
        int c = getopt_long(
            argc, argv, "diw:",
            (program_mode == MODE_DEFAULT) ? basencopts : baseopts,
            &opt_idx
        );
        if (c == -1) {
            break;
        }
        switch (c) {
            case 0:
                break;
            case ENCODING_BASE64:
            case ENCODING_BASE64URL:
            case ENCODING_BASE32:
            case ENCODING_BASE32HEX:
            case ENCODING_BASE16:
            case ENCODING_BASE2MSBF:
            case ENCODING_BASE2LSBF:
            case ENCODING_Z85:
                encoding = c;
                break;
            case 'w': {
                char *endptr = NULL;
                wrap = strtoul(optarg, &endptr, 10);
                if (*endptr) {
                    fprintf(
                        stderr, "%s: invalid wrap size: '%s'\n",
                        __progname, optarg
                    );
                    return 1;
                }
                break;
            }
            case 'h':
               usage(stdout);
               return 0;
           case 'v':
               printf(
"%s (bsdutils-extra) " PROJECT_VERSION "\n"
"Copyright (C) 2021 Daniel Kolesa\n"
"SPDX-License-Identifier: BSD-2-Clause\n",
                    __progname
                );
                return 0;
           default:
               if (optopt == 0) {
                   fprintf(
                       stderr, "%s: unrecognized option '%s'\n",
                       __progname, argv[optind - 1]
                   );
               } else {
                   fprintf(
                       stderr, "%s: invalid option -- '%c'\n",
                       __progname, optopt
                   );
               }
               return 1;
        }
    }

    if (encoding == ENCODING_UNKNOWN) {
        fprintf(stderr, "%s: missing encoding type\n", __progname);
        fprintf(stderr, "Try '%s --help' for more information.\n", __progname);
        return 1;
    }

    FILE *fstream;

    if (optind >= argc) {
        fstream = stdin;
        fpath = "stdin";
    } else if (optind == (argc - 1)) {
        fpath = argv[optind];
        fstream = fopen(fpath, "r");
        if (!fstream) {
            fprintf(stderr, "%s: ", __progname);
            perror(argv[optind]);
            return 1;
        }
    } else {
        fprintf(
            stderr, "%s: extra operand '%s'\n", __progname, argv[optind + 1]
        );
        return 1;
    }

    char *fbuf = malloc(IBUFSIZE + OBUFSIZE);
    if (!fbuf) {
        fprintf(stderr, "%s: out of memory\n", __progname);
        return 1;
    }

    /* never wrap when decoding */
    if (opt_decode) {
        wrap = 0;
    }

    switch (encoding) {
        case ENCODING_BASE64:
            base_basenc = opt_decode ? base64_decode : base64_encode;
            base_alpha = b64_alpha;
            base_dtbl = b64_dtbl;
            break;
        case ENCODING_BASE64URL:
            base_basenc = opt_decode ? base64_decode : base64_encode;
            base_alpha = b64url_alpha;
            base_dtbl = b64url_dtbl;
            break;
        case ENCODING_BASE32:
            base_basenc = opt_decode ? base32_decode : base32_encode;
            base_alpha = b32_alpha;
            base_dtbl = b32_dtbl;
            break;
        case ENCODING_BASE32HEX:
            base_basenc = opt_decode ? base32_decode : base32_encode;
            base_alpha = b32hex_alpha;
            base_dtbl = b32hex_dtbl;
            break;
        case ENCODING_BASE16:
            base_basenc = opt_decode ? base16_decode : base16_encode;
            base_alpha = b16_alpha;
            base_dtbl = b16_dtbl;
            break;
        case ENCODING_BASE2MSBF:
            base_basenc = opt_decode ? base2msbf_decode : base2msbf_encode;
            break;
        case ENCODING_BASE2LSBF:
            base_basenc = opt_decode ? base2lsbf_decode : base2lsbf_encode;
            break;
        case ENCODING_Z85:
            base_basenc = opt_decode ? z85_decode : z85_encode;
            base_alpha = z85_alpha;
            base_dtbl = z85_dtbl;
            break;
        default:
            /* unreachable */
            abort();
    }

    int retcode = 0;

    /* disable buffering when not in tty and not wrapping the output,
     * we are using our own and dumping it all at once when needed
     */
    if (!isatty(1) && (wrap == 0)) {
        setvbuf(stdout, NULL, _IONBF, 0);
    }

    if (!do_basenc(fstream, fbuf, fbuf + IBUFSIZE, fpath)) {
        retcode = 1;
    }

    if (dec_failed) {
        fprintf(stderr, "%s: invalid input\n", __progname);
        retcode = 1;
    }

    if (fstream != stdin) {
        fclose(fstream);
    }

    free(fbuf);

    if (!opt_decode) {
        fputc('\n', stdout);
    }

    return retcode;
}
