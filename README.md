# chimerautils

This is Chimera Linux's core userland. It consists of the following:

* Ports of FreeBSD tools
* An internal library providing a compat interface to simplify porting
* Custom-made new tools
* A Meson-based build system

It replaces the following GNU projects:

* coreutils
* findutils
* diffutils
* sharutils
* grep
* sed
* ed
* m4
* patch
* gzip
* gawk
* bc (optional, bc-gh is recommended now)

It also provides the following functionality:

* tip/cu
* telnet
* fetch
* gencat
* nc
* vi
* sh
* vis
* unvis
* compress
* uncompress
* portions of util-linux
* and additional custom tools

In a way, `chimerautils` is also an alternative to projects like Busybox.

## bsdutils

This project is a fork of [bsdutils](https://github.com/dcantrell/bsdutils)
by David Cantrell. Chimerautils were created in order to provide a more
complete package that prioritizes Chimera's needs and development pace.

## Building

Chimerautils requires a Linux system with a Clang or GCC compiler.

You will also need the following:

* `meson` and `ninja`
* `flex` (or another `lex`)
* `byacc` (or `bison`)
* `libxo` (https://github.com/Juniper/libxo)

Optionally, these are also needed:

* `ncurses` or another provider of `terminfo` (for color `ls(1)` and others)
* `libedit` (for `bc` and line editing in `sh`)
* `libcrypto` from OpenSSL or LibreSSL (for `dc`, `install` and optionally `sort`)

If your C library does not provide them, you will need these:

* `libfts`
* `librpmatch`

To build:

```
$ mkdir build && cd build
$ meson ..
$ ninja all
```

## Importing a new FreeBSD release

When a new release of FreeBSD is made, the import-src.sh script should
be used to update the source tree.  First edit upstream.conf and then
run the import-src.sh script.  The script will fetch the new release
source and copy in the source for the commands we have.  Any patches
in patches/ will be applied.  These may need updating between
releases, so keep that in mind.  The workflow is basically:

1) Change VER in upstream.conf

2) Verify URL in upstream.conf works (FreeBSD may move things around).

3) Run ./import-src.sh.  It is adviseable to capture stdout and stderr
to see what patches fail to apply.  Any that fail, you want to
manually fix and then run import-src.sh again to get a clean import of
the version you are updating to.

4) Now build all the commands and fix any new build errors.

Once this is clean, you can commit the import of the new version of
FreeBSD code.  The import-src.sh and patches step is meant to make it
more clear what changes I apply to FreeBSD code from release to
release and also if any external projects want to use these patches
and the FreeBSD source directly.
