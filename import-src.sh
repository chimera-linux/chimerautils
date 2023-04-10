#!/bin/sh
#
# import-src.sh - Import specific release of FreeBSD source in to
#                 this tree.  Primarily for maintenance use when
#                 a new version of FreeBSD comes out.
#
# Author: David Cantrell <david.l.cantrell@gmail.com>
#

PATH=/bin:/usr/bin
CWD="$(pwd)"
TMPDIR="$(mktemp -d --tmpdir=${CWD})"
. ${CWD}/upstream.conf

fail_exit() {
    cd ${CWD}
    rm -rf ${TMPDIR}
    exit 1
}

rm -rf src.orig
mkdir -p src.orig src.freebsd

SRCTAR=
if [ -n "$1" ]; then
    [ -r "$1" ] || fail_exit
    SRCTAR=$(realpath "$1")
fi

cd ${TMPDIR}
if [ -z "$SRCTAR" ]; then
    SRCTAR="src.txz"
    curl -L --retry 3 --ftp-pasv -O ${SRC} || fail_exit
fi
xz -dc "$SRCTAR" | tar -xf -

copy_cmd() {
    p="$1"
    sd="$2"
    dn="$3"
    rp="usr/src/${p}"
    sp="$(basename ${p})"
    [ -z "$dn" ] && dn="$sp"
    if [ -n "$sd" ]; then
        dp="${sd}/${dn}"
    else
        dp="$dn"
    fi

    # Drop the tests/ subdirectories
    [ -d ${rp}/tests ] && rm -rf ${rp}/tests

    # Rename the upstream Makefile for later manual checking.  We don't
    # commit these to our tree, but just look at them when rebasing and
    # pick up any rule changes to put in our Makefile.am files.
    if [ -f "${rp}/Makefile" ]; then
        mv ${rp}/Makefile ${rp}/Makefile.bsd
    fi

    # Drop the Makefile.depend* files
    rm -f ${rp}/Makefile.depend*

    # Copy in the upstream files
    [ -d ${CWD}/src.orig/${dp} ] || mkdir -p ${CWD}/src.orig/${dp}
    [ -d ${CWD}/src.freebsd/${dp} ] || mkdir -p ${CWD}/src.freebsd/${dp}
    cp -pr ${rp}/* ${CWD}/src.orig/${dp}
    cp -pr ${rp}/* ${CWD}/src.freebsd/${dp}
}

# coreutils
CMDS_CORE="
bin/cat
bin/chmod
bin/cp
bin/date
bin/dd
bin/df
bin/echo
bin/expr
bin/hostname
bin/ln
bin/ls
bin/mkdir
bin/mv
bin/pwd
bin/realpath
bin/rm
bin/rmdir
bin/sleep
bin/stty
bin/sync
bin/test
bin/timeout
sbin/mknod
usr.bin/basename
usr.bin/cksum
usr.bin/comm
usr.bin/cut
usr.bin/csplit
usr.bin/dirname
usr.bin/du
usr.bin/env
usr.bin/expand
usr.bin/factor
usr.bin/false
usr.bin/fmt
usr.bin/fold
usr.bin/head
usr.bin/id
usr.bin/join
usr.bin/logname
usr.bin/mktemp
usr.bin/mkfifo
usr.bin/nice
usr.bin/nl
usr.bin/nohup
usr.bin/paste
usr.bin/pathchk
usr.bin/pr
usr.bin/printenv
usr.bin/printf
usr.bin/seq
usr.bin/sort
usr.bin/split
usr.bin/stat
usr.bin/stdbuf
usr.bin/tail
usr.bin/tee
usr.bin/touch
usr.bin/tr
usr.bin/true
usr.bin/truncate
usr.bin/tsort
usr.bin/tty
usr.bin/uname
usr.bin/unexpand
usr.bin/uniq
usr.bin/users
usr.bin/wc
usr.bin/who
usr.bin/yes
usr.bin/xinstall
usr.sbin/chown
usr.sbin/chroot
"

# diffutils
CMDS_DIFF="
usr.bin/cmp
usr.bin/diff
usr.bin/sdiff
"

# findutils
CMDS_FIND="
usr.bin/find
usr.bin/xargs
"
CMDS_FIND_LOCATE="
usr.bin/locate/bigram
usr.bin/locate/code
usr.bin/locate/locate
"

# bc
CMDS_BC="
usr.bin/bc
usr.bin/dc
"

# mostly util-linux
CMDS_MISC="
bin/kill
usr.bin/calendar
usr.bin/col
usr.bin/colrm
usr.bin/column
usr.bin/getopt
usr.bin/hexdump
usr.bin/logger
usr.bin/look
usr.bin/mesg
usr.bin/ncal
usr.bin/renice
usr.bin/rev
usr.bin/script
usr.bin/ul
usr.bin/wall
usr.bin/whereis
usr.bin/write
"

for p in ${CMDS_CORE}; do
    copy_cmd "$p" coreutils
done

for p in ${CMDS_DIFF}; do
    copy_cmd "$p" diffutils
done

for p in ${CMDS_FIND}; do
    copy_cmd "$p" findutils
done

for p in ${CMDS_FIND_LOCATE}; do
    copy_cmd "$p" findutils/locate
done

for p in ${CMDS_BC}; do
    copy_cmd "$p" bc
done

for p in ${CMDS_MISC}; do
    copy_cmd "$p" miscutils
done

# equivalents of standalone projects
copy_cmd bin/ed
copy_cmd bin/sh
copy_cmd usr.bin/grep
copy_cmd usr.bin/gzip
copy_cmd usr.bin/m4
copy_cmd usr.bin/patch
copy_cmd usr.bin/sed
copy_cmd usr.bin/which
copy_cmd contrib/nvi
copy_cmd contrib/one-true-awk "" awk

# 'compat' is our static library with a subset of BSD library functions
mkdir -p ${CWD}/src.orig/compat ${CWD}/src.orig/include
cp -p usr/src/lib/libutil/expand_number.c ${CWD}/src.orig/compat
cp -p usr/src/lib/libc/gen/getbsize.c ${CWD}/src.orig/compat
cp -p usr/src/lib/libc/stdlib/heapsort.c ${CWD}/src.orig/compat
cp -p usr/src/lib/libutil/humanize_number.c ${CWD}/src.orig/compat
cp -p usr/src/lib/libc/stdlib/merge.c ${CWD}/src.orig/compat
cp -p usr/src/lib/libopenbsd/ohash.c ${CWD}/src.orig/compat
cp -p usr/src/lib/libc/gen/setmode.c ${CWD}/src.orig/compat
cp -p usr/src/lib/libc/string/strmode.c ${CWD}/src.orig/compat
cp -p usr/src/lib/libc/gen/stringlist.c ${CWD}/src.orig/compat
cp -p usr/src/lib/libc/stdtime/strptime.c ${CWD}/src.orig/compat
cp -p usr/src/lib/libc/stdtime/strftime.c ${CWD}/src.orig/compat
cp -p usr/src/contrib/libc-vis/vis.c ${CWD}/src.orig/compat
cp -p usr/src/include/stringlist.h ${CWD}/src.orig/include
cp -p usr/src/contrib/libc-vis/vis.h ${CWD}/src.orig/include
cp -p usr/src/lib/libopenbsd/ohash.h ${CWD}/src.orig/include

mkdir -p ${CWD}/src.freebsd/compat ${CWD}/src.freebsd/include
cp -p ${CWD}/src.orig/compat/* ${CWD}/src.freebsd/compat
cp -p ${CWD}/src.orig/include/* ${CWD}/src.freebsd/include

# 'dbcompat' is our subset of berkeley db 1.85
mkdir -p ${CWD}/src.orig/dbcompat/btree ${CWD}/src.orig/dbcompat/recno ${CWD}/src.orig/dbinclude
cp -p usr/src/lib/libc/db/db/db.c ${CWD}/src.orig/dbcompat
cp -p usr/src/lib/libc/db/mpool/mpool.c ${CWD}/src.orig/dbcompat
cp -p usr/src/lib/libc/db/btree/*.[ch] ${CWD}/src.orig/dbcompat/btree
cp -p usr/src/lib/libc/db/recno/*.[ch] ${CWD}/src.orig/dbcompat/recno
cp -p usr/src/include/db.h ${CWD}/src.orig/dbinclude
cp -p usr/src/include/mpool.h ${CWD}/src.orig/dbinclude

mkdir -p ${CWD}/src.freebsd/dbcompat ${CWD}/src.freebsd/dbinclude
cp -pr ${CWD}/src.orig/dbcompat/* ${CWD}/src.freebsd/dbcompat
cp -pr ${CWD}/src.orig/dbinclude/* ${CWD}/src.freebsd/dbinclude

# These files are needed for the factor command
cp -p usr/src/usr.bin/primes/primes.h ${CWD}/src.orig/coreutils/factor
cp -p usr/src/usr.bin/primes/pr_tbl.c ${CWD}/src.orig/coreutils/factor
cp -p usr/src/usr.bin/primes/primes.h ${CWD}/src.freebsd/coreutils/factor
cp -p usr/src/usr.bin/primes/pr_tbl.c ${CWD}/src.freebsd/coreutils/factor

# These are not used
rm -rf ${CWD}/src.orig/coreutils/sort/nls
rm -rf ${CWD}/src.freebsd/coreutils/sort/nls

# sort manpage
mv ${CWD}/src.orig/coreutils/sort/sort.1.in ${CWD}/src.orig/coreutils/sort/sort.1
mv ${CWD}/src.freebsd/coreutils/sort/sort.1.in ${CWD}/src.freebsd/coreutils/sort/sort.1

# libcalendar internal copy for ncal(1)
cp -p usr/src/lib/libcalendar/easter.c ${CWD}/src.orig/miscutils/ncal/easter.c
cp -p usr/src/lib/libcalendar/calendar.c ${CWD}/src.orig/miscutils/ncal/calendar.c
cp -p usr/src/lib/libcalendar/calendar.h ${CWD}/src.orig/miscutils/ncal/calendar.h
cp -p usr/src/lib/libcalendar/easter.c ${CWD}/src.freebsd/miscutils/ncal/easter.c
cp -p usr/src/lib/libcalendar/calendar.c ${CWD}/src.freebsd/miscutils/ncal/calendar.c
cp -p usr/src/lib/libcalendar/calendar.h ${CWD}/src.freebsd/miscutils/ncal/calendar.h

# fix sh generator permissions
chmod 755 ${CWD}/src.orig/sh/mkbuiltins
chmod 755 ${CWD}/src.orig/sh/mktokens
chmod 755 ${CWD}/src.freebsd/sh/mkbuiltins
chmod 755 ${CWD}/src.freebsd/sh/mktokens

# remove sh files we don't want
rm -rf ${CWD}/src.orig/sh/dot.*
rm -rf ${CWD}/src.orig/sh/funcs
rm -f ${CWD}/src.orig/sh/profile
rm -rf ${CWD}/src.freebsd/sh/dot.*
rm -rf ${CWD}/src.freebsd/sh/funcs
rm -f ${CWD}/src.freebsd/sh/profile

# remove our own nvi scripts before patching
rm -f ${CWD}/src.freebsd/nvi/*.sh

#####################
# APPLY ANY PATCHES #
#####################

cd ${CWD}/patches

for p in *.patch; do
    [ -f "$p" ] || continue
    patch -d ${CWD}/src.freebsd -p1 < $p
done

# fix up permissions
chmod 755 ${CWD}/src.freebsd/nvi/*.sh

# Clean up
rm -rf ${TMPDIR}
