#!/bin/sh
# since the generators always emit to current directory and meson has
# no way to set the current directory used during the call, just do this
cd "$1"
shift
exec "$@"
