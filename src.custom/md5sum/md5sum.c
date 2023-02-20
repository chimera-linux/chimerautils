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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libgen.h>
#include <getopt.h>

#include <openssl/evp.h>

enum mode {
    MODE_UNKNOWN = 0,
    MODE_MD5,
    MODE_BLAKE2,
    MODE_SHA1,
    MODE_SHA224,
    MODE_SHA256,
    MODE_SHA384,
    MODE_SHA512,
#if 0
    /* legacy provider in openssl 3.x */
    MODE_RMD160
#endif
};

enum style {
    STYLE_UNKNOWN = 0,
    STYLE_GNU,
    STYLE_BSD
};

static int opt_binary, opt_check,
           opt_quiet, opt_status, opt_warn, opt_stdin,
           opt_reverse, opt_datastr, opt_help, opt_version;

static struct option gnuopts[] = {
    {"binary",         no_argument, &opt_binary,         1},
    {"check",          no_argument, &opt_check,          1},
    {"text",           no_argument, &opt_binary,         0},
    {"quiet",          no_argument, &opt_quiet,          1},
    {"status",         no_argument, &opt_status,         1},
    {"warn",           no_argument, &opt_warn,           1},
    {"help",           no_argument, &opt_help,           1},
    {"version",        no_argument, &opt_version,        1},
    {0, 0, 0, 0}
};

static const char *shopts_gnu = "bctw";
static const char *shopts_bsd = "c:s:pqr";

#define BUFSIZE (16 * 1024)

extern const char *__progname;

static unsigned char digest[EVP_MAX_MD_SIZE];
static unsigned int digestsize;

static void usage_gnu(FILE *stream, const char *bname, unsigned int dgsize) {
    fprintf(stream,
"Usage: %s [OPTION]... [FILE]...\n"
"Print or check %s (%u-bit) checksums.\n"
"\n"
"With no FILE, or when FILE is -, read standard input.\n"
"\n"
"  -b, --binary         read in binary mode\n"
"  -c, --check          read %s sums from the FILEs and check them\n"
"  -t, --text           read in text mode (default)\n"
"\n"
"The following five options are useful only when verifying checksums:\n"
"      --quiet          don't print OK for each successfully verified file\n"
"      --status         don't output anything, status code shows success\n"
"  -w, --warn           warn about improperly formatted checksum lines\n"
"\n"
"      --help     display this help and exit\n"
"      --version  output version information and exit\n"
"\n"
"The sums are computed as described in RFC 7693.  When checking, the input\n"
"should be a former output of this program.  The default mode is to print a\n"
"line with checksum, a space, a character indicating input mode ('*' for binary,\n"
"' ' for text or where binary is insignificant), and name for each FILE.\n"
"\n"
"Note: The binary and text mode switch only exists for compatibility reasons.\n",
        __progname, bname, dgsize, bname
    );
}

static void usage_bsd(FILE *stream) {
    fprintf(
        stream, "usage: %s [-pqrtx] [-c string] [-s string] [files ...]\n",
        __progname
    );
}

#define HEX_DIGIT(c) (unsigned char)((c > 57) ? ((c | 32) - 87) : (c - 48))

static int digest_compare(
    unsigned char *dstr, unsigned int mdlen, const char *cmp
) {
    for (unsigned int i = 0; i < mdlen; ++i) {
        if (((HEX_DIGIT(cmp[0]) << 4) | HEX_DIGIT(cmp[1])) != dstr[i]) {
            return 0;
        }
        cmp += 2;
    }
    return 1;
}

static char *get_basename(char *path) {
    char *tslash = strrchr(path, '/');
    if (!tslash) {
        return path;
    }
    if (strlen(tslash + 1) == 0) {
        *tslash = '\0';
        return get_basename(path);
    }
    return tslash + 1;
}

static int handle_file(
    const char *fname, FILE *stream, char *rbuf, const EVP_MD *md,
    EVP_MD_CTX *ctx, int hstyle, const char *bname, const char *cmp
) {
    if (opt_check && hstyle == STYLE_GNU) {
        opt_check = 0;
        char *buf = NULL;
        size_t nc = 0;
        size_t linenum = 1;
        size_t nbadlines = 0;
        size_t nfailread = 0;
        while (getline(&buf, &nc, stream) >= 0) {
            char *dstr = buf;
            char *cfname = strstr(buf, "  ");
            if (cfname) {
                *cfname = '\0';
            }
            /* validate the digest */
            int isdigest = 1;
            if (!cfname || ((strlen(dstr) * 4) != digestsize)) {
                isdigest = 0;
            }
            if (isdigest) {
                for (unsigned int i = 0; i < (digestsize / 4); ++i) {
                    if (!isxdigit(dstr[i])) {
                        isdigest = 0;
                        break;
                    }
                }
            }
            if (!isdigest) {
                if (opt_warn) {
                    fprintf(
                        stderr,
                        "%s: %s: %zu: improperly formatted %s checksum line\n",
                        __progname, fname, linenum, bname
                    );
                }
                ++nbadlines;
                ++linenum;
                continue;
            }
            cfname += 2;
            char *nl = strchr(cfname, '\n');
            if (nl) {
                *nl = '\0';
            }
            ++linenum;
            FILE *f = fopen(cfname, "r");
            if (!f) {
                fprintf(stderr, "%s: ", __progname);
                perror(cfname);
                fprintf(stderr, "%s: FAILED open or read\n", cfname);
                ++nfailread;
                continue;
            }
            int ret = handle_file(
                cfname, f, rbuf, md, ctx, hstyle, bname, dstr
            );
            if (ret == 255) {
                fprintf(stderr, "%s: FAILED\n", cfname);
                continue;
            } else if (ret) {
                fprintf(stderr, "%s: FAILED open or read\n", cfname);
                ++nfailread;
                continue;
            } else if (!opt_quiet) {
                printf("%s: OK\n", cfname);
            }
        }
        if (nbadlines) {
            fprintf(
                stderr, "%s: WARNING: %zu lines are improperly formatted\n",
                __progname, nbadlines
            );
        }
        if (nfailread) {
            fprintf(
                stderr, "%s: WARNING: %zu listed files could not be read\n",
                __progname, nfailread
            );
        }
        opt_check = 1;
        free(buf);
        return 0;
    }

    EVP_MD_CTX_reset(ctx);

    if (!EVP_DigestInit_ex(ctx, md, NULL)) {
        fprintf(stderr, "%s: failed to initialize digest\n", __progname);
        return 1;
    }

    if (stream) {
        for (;;) {
            size_t n = fread(rbuf, 1, BUFSIZE, stream);
            if (n != BUFSIZE) {
                if (feof(stream)) {
                    if (opt_stdin && (stream == stdin)) {
                        fwrite(rbuf, 1, n, stdout);
                    }
                    EVP_DigestUpdate(ctx, rbuf, n);
                    break;
                }
                if (ferror(stream)) {
                    char *mfname = strdup(fname);
                    fprintf(stderr, "%s: ", __progname);
                    perror(get_basename(mfname));
                    free(mfname);
                    return 1;
                }
            } else {
                if (opt_stdin && (stream == stdin)) {
                    fwrite(rbuf, 1, BUFSIZE, stdout);
                }
                EVP_DigestUpdate(ctx, rbuf, BUFSIZE);
            }
        }
    } else {
        /* no stream: assume fname is the string we are checking */
        EVP_DigestUpdate(ctx, fname, strlen(fname));
    }

    unsigned int mdlen = 0;
    if (!EVP_DigestFinal_ex(ctx, digest, &mdlen)) {
        fprintf(stderr, "%s: failed to finalize digest\n", __progname);
        return 1;
    }

    if (cmp && hstyle == STYLE_GNU) {
        if (!digest_compare(digest, mdlen, cmp)) {
            return 255;
        }
        return 0;
    }

    if ((hstyle == STYLE_BSD) && !opt_reverse && !opt_quiet && stream != stdin) {
        if (!stream) {
            printf("%s (\"%s\") = ", bname, fname);
        } else {
            printf("%s (%s) = ", bname, fname);
        }
    }

    for (unsigned int i = 0; i < mdlen; ++i) {
        printf("%02x", digest[i]);
    }

    if (hstyle == STYLE_GNU) {
        printf("  %s", fname);
    } else if (opt_reverse && (stream != stdin)) {
        if (!stream) {
            printf(" \"%s\"", fname);
        } else {
            printf(" %s", fname);
        }
    }

    if ((hstyle == STYLE_BSD) && cmp) {
        int isdigest = 1;
        /* validate digest */
        if ((strlen(cmp) * 4) != digestsize) {
            isdigest = 0;
        }
        if (isdigest) {
            for (unsigned int i = 0; i < (digestsize / 4); ++i) {
                if (!isxdigit(cmp[i])) {
                    isdigest = 0;
                    break;
                }
            }
        }
        if (isdigest) {
            isdigest = digest_compare(digest, mdlen, cmp);
        }
        if (!isdigest) {
            if (!opt_quiet && (stream != stdin)) {
                printf(" [ Failed ]\n");
            } else {
                printf("\n");
            }
            return 2;
        }
    }

    printf("\n");
    return 0;
}

int main(int argc, char **argv) {
    enum mode hmode = MODE_UNKNOWN;
    enum style hstyle = STYLE_UNKNOWN;
    const char *scmp = NULL;
    const char *bname = NULL;
    const char *checkstr = NULL;
    const char *datastr = NULL;

    if (!strcmp(__progname, "b2sum")) {
        hmode = MODE_BLAKE2;
        bname = "BLAKE2";
        hstyle = STYLE_GNU;
        digestsize = 512;
    } else if (!strncmp(__progname, "sha1", 4)) {
        bname = "SHA1";
        hmode = MODE_SHA1;
        scmp = __progname + 4;
        digestsize = 160;
    } else if (!strncmp(__progname, "sha224", 6)) {
        bname = "SHA224";
        hmode = MODE_SHA224;
        scmp = __progname + 6;
        digestsize = 224;
    } else if (!strncmp(__progname, "sha256", 6)) {
        bname = "SHA256";
        hmode = MODE_SHA256;
        scmp = __progname + 6;
        digestsize = 256;
    } else if (!strncmp(__progname, "sha384", 6)) {
        bname = "SHA384";
        hmode = MODE_SHA384;
        scmp = __progname + 6;
        digestsize = 384;
    } else if (!strncmp(__progname, "sha512", 6)) {
        bname = "SHA512";
        hmode = MODE_SHA512;
        scmp = __progname + 6;
        digestsize = 512;
#if 0
    } else if (!strcmp(__progname, "rmd160")) {
        bname = "RMD160";
        hmode = MODE_RMD160;
        hstyle = STYLE_BSD;
        digestsize = 160;
#endif
    }

    if ((hstyle == STYLE_UNKNOWN) && scmp) {
        if (!*scmp) {
            hstyle = STYLE_BSD;
        } else if (!strcmp(scmp, "sum")) {
            hstyle = STYLE_GNU;
        }
    }

    /* with unknown progname, pretend we're md5sum */
    if (hmode == MODE_UNKNOWN || hstyle == STYLE_UNKNOWN) {
        hmode = MODE_MD5;
        hstyle = STYLE_GNU;
    }

    opterr = 0;

    for (;;) {
        int c;
        int opt_idx = 0;
        if (hstyle == STYLE_GNU) {
            c = getopt_long(argc, argv, shopts_gnu, gnuopts, &opt_idx);
        } else {
            c = getopt(argc, argv, shopts_bsd);
        }
        if (c == -1) {
            break;
        }
        switch (c) {
            case 0:
                if (hstyle == STYLE_BSD) {
                    /* should be unreacahble */
                    abort();
                }
                /* we have flags, nothing to do */
                break;
            case 'b':
            case 't':
                break;
            case 'c':
                opt_check = 1;
                if (hstyle == STYLE_BSD) {
                    checkstr = optarg;
                }
                break;
            case 's':
                opt_datastr = 1;
                datastr = optarg;
                break;
            case 'w':
                opt_warn = 1;
                break;
            case 'p':
                opt_stdin = 1;
                break;
            case 'q':
                opt_quiet = 1;
                break;
            case 'r':
                opt_reverse = 1;
                break;
            default:
                if (hstyle == STYLE_BSD) {
                    fprintf(stderr, "%s: illegal option -- %c\n", __progname, c);
                    usage_bsd(stderr);
                    return 1;
                } else {
                    fprintf(
                        stderr, "%s: unrecognized option '-%c'\n",
                        __progname, c
                    );
                    fprintf(
                        stderr, "Try '%s --help' for more information.\n",
                        __progname
                    );
                    return 1;
                }
        }
    }

    if (opt_help) {
        usage_gnu(stdout, bname, digestsize);
        return 0;
    } else if (opt_version) {
        printf(
"%s (" PROJECT_NAME ") " PROJECT_VERSION "\n"
"Copyright (C) 2021 Daniel Kolesa\n"
"SPDX-License-Identifier: BSD-2-Clause\n",
            __progname
        );
        return 0;
    }

    const EVP_MD *md = NULL;
    switch (hmode) {
        case MODE_BLAKE2:
            md = EVP_blake2b512();
            break;
        case MODE_MD5:
            md = EVP_md5();
            break;
        case MODE_SHA1:
            md = EVP_sha1();
            break;
        case MODE_SHA224:
            md = EVP_sha224();
            break;
        case MODE_SHA256:
            md = EVP_sha256();
            break;
        case MODE_SHA384:
            md = EVP_sha384();
            break;
        case MODE_SHA512:
            md = EVP_sha512();
            break;
#if 0
        case MODE_RMD160:
            md = EVP_ripemd160();
            break;
#endif
        default:
            break;
    }
    if (!md) {
        fprintf(stderr, "%s: failed to initialize digest\n", __progname);
        return 1;
    }

    char *rbuf = malloc(BUFSIZE);
    if (!rbuf) {
        fprintf(stderr, "%s: failed to allocate memory\n", __progname);
        return 1;
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) {
        free(rbuf);
        fprintf(stderr, "%s: failed to initialize digest context\n", __progname);
        return 1;
    }

    if (opt_datastr) {
        int rval = handle_file(
            datastr, NULL, rbuf, md, ctx, hstyle, bname, checkstr
        );
        if (rval) {
            free(rbuf);
            EVP_MD_CTX_free(ctx);
            return rval;
        }
    }

    if (optind >= argc) {
        if (opt_datastr) {
            return 0;
        }
        int rval = handle_file(
            "stdin", stdin, rbuf, md, ctx, hstyle, bname, checkstr
        );
        if (rval) {
            free(rbuf);
            EVP_MD_CTX_free(ctx);
            return rval;
        }
    } else {
        while (optind < argc) {
            const char *fname = argv[optind++];
            FILE *f = stdin;
            if (strcmp(fname, "-")) {
                f = fopen(fname, "r");
            }
            if (!f) {
                free(rbuf);
                EVP_MD_CTX_free(ctx);
                char *mfname = strdup(fname);
                fprintf(stderr, "%s: ", __progname);
                perror(get_basename(mfname));
                free(mfname);
                return 1;
            }
            int rval = handle_file(
                fname, f, rbuf, md, ctx, hstyle, bname, checkstr
            );
            fclose(f);
            if (rval) {
                free(rbuf);
                EVP_MD_CTX_free(ctx);
                return rval;
            }
        }
    }

    return 0;
}
