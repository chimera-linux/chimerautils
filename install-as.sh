#!/bin/sh
# this is a workaround for meson not being able to install
# binaries with reserved names, in our case 'test' and 'install'

dstp="${DESTDIR}/${MESON_INSTALL_PREFIX}/$1"
srcf="$2"
dstf="$3"
shift 3

install -d "$dstp"
install -m 0755 "$srcf" "${dstp}/${dstf}"

while [ "$#" -gt 0 ]; do
    ln -sf "$dstf" "${dstp}/$1"
    shift
done
