/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2024 q66 <q66@chimera-linux.org>
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

#include <filesystem>
#include <utility>
#include <cstdio>
#include <cstdlib>
#include <cerrno>

#include <unistd.h>
#include <getopt.h>
#include <err.h>

enum {
    ARG_RELATIVE_TO = 127,
    ARG_HELP,
    ARG_VERSION,
};

namespace fs = std::filesystem;

static bool canonical_missing = false;
static bool quiet = false;
static bool strip = false;
static bool zero = false;
static bool isrel = false;
static fs::path relpath{};

extern char const *__progname;

static void usage_readlink(bool help) {
    std::fprintf(
        help ? stdout : stderr,
        "Usage: %s [OPTION]... FILE...\n"
        "Print value of a symbolic link or canonical file name\n"
        "\n"
        "  -f, --canonicalize\n"
        "  -e, --canonicalize-existing  canonicalize by following every symlink\n"
        "                               in every component of the given name\n"
        "                               recursively, all components must exist\n"
        "  -m, --canonicalize-missing   canonicalize by following every symlink\n"
        "                               in every component of the given name\n"
        "                               recursively, no component must exist\n"
        "  -n, --no-newline             do not output the trailing newline\n"
        "  -q, --quiet\n"
        "  -s, --silent                 suppress most error messages (default)\n"
        "  -v, --verbose                do not suppress error messages\n"
        "  -z, --zero                   delimit with NUL instead of newline\n"
        "      --help                   print this help message\n"
        "      --version                print the version\n",
        __progname
    );
}

static void usage_realpath(bool help) {
    std::fprintf(
        help ? stdout : stderr,
        "Usage: %s [OPTION]... FILE...\n"
        "Print the resolved absolute file name\n"
        "\n"
        "By default, all components must exist.\n"
        "\n"
        "  -e, --canonicalize-existing  all components must exist (default)\n"
        "  -m, --canonicalize-missing   no component must exist\n"
        "  -s, --strip, --no-symlinks   don't expand symlinks, only normalize\n"
        "      --relative-to=DIR        print result reslative to DIR\n"
        "  -q, --quiet                  suppress most error messages\n"
        "  -z, --zero                   delimit with NUL instead of newline\n"
        "      --help                   print this help message\n"
        "      --version                print the version\n",
        __progname
    );
}

static bool do_realpath(fs::path sp, bool newl) {
    fs::path np;
    std::error_code ec{};
    /* then do the actual resolution */
    if (strip && sp.is_relative()) {
        /* no symlinks are expanded + relative input */
        np = (fs::current_path(ec) / sp).lexically_normal();
    } else if (strip) {
        /* no symlinks are expanded + already absolute */
        np = sp.lexically_normal();
    } else if (canonical_missing) {
        /* no components need to exist */
        np = fs::weakly_canonical(sp, ec);
    } else {
        /* all components must exist */
        np = fs::canonical(sp, ec);
    }
    if (ec) {
        errno = ec.value();
        if (!quiet) {
            warn("%s", sp.c_str());
        }
        return false;
    }
    /* process */
    if (isrel) {
        np = np.lexically_relative(relpath);
    }
    auto cstr = np.c_str();
    write(STDOUT_FILENO, cstr, std::strlen(cstr));
    if (!newl) {
        return true;
    }
    if (zero) {
        write(STDOUT_FILENO, "\0", 1);
    } else {
        write(STDOUT_FILENO, "\n", 1);
    }
    return true;
}

static int readlink_main(int argc, char **argv) {
    struct option lopts[] = {
        {"canonicalize", no_argument, 0, 'f'},
        {"canonicalize-existing", no_argument, 0, 'e'},
        {"canonicalize-missing", no_argument, 0, 'm'},
        {"no-newline", no_argument, 0, 'n'},
        {"quiet", no_argument, 0, 'q'},
        {"silent", no_argument, 0, 's'},
        {"verbose", no_argument, 0, 'v'},
        {"zero", no_argument, 0, 'z'},
        {"help", no_argument, 0, ARG_HELP},
        {"version", no_argument, 0, ARG_VERSION},
        {nullptr, 0, 0, 0},
    };

    /* readlink behavior */
    bool canonical = false;
    bool newl = true;
    quiet = true;

    for (;;) {
        int oind = 0;
        auto c = getopt_long(argc, argv, "femnqsvz", lopts, &oind);
        if (c < 0) {
            break;
        }
        switch (c) {
            case 'f':
            case 'e':
            case 'm':
                canonical = true;
                canonical_missing = (c == 'm');
                break;
            case 'n':
                newl = false;
                break;
            case 'q':
            case 's':
                quiet = true;
                break;
            case 'v':
                quiet = false;
                break;
            case 'z':
                zero = true;
                break;
            case ARG_HELP:
                usage_readlink(true);
                return 0;
            case ARG_VERSION:
                std::printf("readlink (" PROJECT_NAME ") " PROJECT_VERSION "\n");
                return 0;
            default:
                usage_realpath(false);
                return 1;
        }
    }

    if (optind >= argc) {
        errx(1, "multiple arguments required");
    }

    int ret = 0;

    /* realpath-like */
    if (canonical) {
        while (optind < argc) {
            auto *p = argv[optind++];
            if (!do_realpath(p, newl || (optind < argc))) {
                ret = 1;
            }
            if (!newl && (optind >= argc)) {
                break;
            }
        }
        return ret;
    }

    while (optind < argc) {
        std::error_code ec{};
        auto sl = fs::read_symlink(argv[optind++], ec);
        if (ec) {
            errno = ec.value();
            if (!quiet) {
                warn("%s", sl.c_str());
            }
            ret = 1;
            continue;
        }
        auto cstr = sl.c_str();
        write(STDOUT_FILENO, cstr, std::strlen(cstr));
        /* copy the gnu behavior, only don't print newline if one input */
        if (!newl && (optind >= argc)) {
            break;
        }
        if (zero) {
            write(STDOUT_FILENO, "\0", 1);
        } else {
            write(STDOUT_FILENO, "\n", 1);
        }
    }

    return ret;
}

static int realpath_main(int argc, char **argv) {
    struct option lopts[] = {
        {"canonicalize-existing", no_argument, 0, 'e'},
        {"canonicalize-missing", no_argument, 0, 'm'},
        {"strip", no_argument, 0, 's'},
        {"no-symlinks", no_argument, 0, 's'},
        {"relative-to", required_argument, 0, ARG_RELATIVE_TO},
        {"quiet", no_argument, 0, 'q'},
        {"zero", no_argument, 0, 'z'},
        {"help", no_argument, 0, ARG_HELP},
        {"version", no_argument, 0, ARG_VERSION},
        {nullptr, 0, 0, 0},
    };

    char const *relstr = nullptr;

    for (;;) {
        int oind = 0;
        auto c = getopt_long(argc, argv, "emqsz", lopts, &oind);
        if (c < 0) {
            break;
        }
        switch (c) {
            case 'e':
            case 'm':
                canonical_missing = (c == 'm');
                break;
            case 'q':
                quiet = true;
                break;
            case 's':
                strip = true;
                break;
            case 'z':
                zero = true;
                break;
            case ARG_RELATIVE_TO:
                isrel = true;
                relstr = optarg;
                relpath = relstr;
                break;
            case ARG_HELP:
                usage_realpath(true);
                return 0;
            case ARG_VERSION:
                std::printf("realpath (" PROJECT_NAME ") " PROJECT_VERSION "\n");
                return 0;
            default:
                usage_realpath(false);
                return 1;
        }
    }

    if (isrel) {
        std::error_code ec{};
        /* make absolute according to current rules */
        if (strip && relpath.is_relative()) {
            relpath = (fs::current_path(ec) / relpath).lexically_normal();
        } else if (strip) {
            relpath = relpath.lexically_normal();
        } else if (canonical_missing) {
            relpath = fs::weakly_canonical(relpath, ec);
        } else {
            relpath = fs::canonical(relpath, ec);
        }
        if (ec) {
            errno = ec.value();
            err(1, "%s", relstr);
        }
    }

    if (optind >= argc) {
        std::error_code ec{};
        /* no arguments */
        auto cwd = fs::current_path(ec);
        if (ec) {
            errno = ec.value();
            err(1, "fs::current_path");
        }
        return !do_realpath(std::move(cwd), true);
    }

    int ret = 0;

    while (optind < argc) {
        if (!do_realpath(argv[optind++], true)) {
            ret = 1;
        }
    }

    return ret;
}

int main(int argc, char **argv) {
    try {
        if (!std::strcmp(__progname, "readlink")) {
            return readlink_main(argc, argv);
        }
        return realpath_main(argc, argv);
    } catch (std::bad_alloc const &) {
        errno = ENOMEM;
        err(1, "alloc");
    }
}
