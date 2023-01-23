#!/bin/sh
#
# this just generates a clean patch between src.orig and src.freebsd

diff -Naur -x meson.build -x '*.orig' src.orig src.freebsd | \
    sed -e '/^diff -Naur/d' \
        -e 's/^\([+-][+-][+-][[:space:]][a-zA-Z0-9/._]*\).*/\1/g'
