#!/bin/sh

COMPONENT=$1
shift

do_sed() {
    sed -n 's/^ \* PUBLIC: \(.*\)/\1/p' "$@"
}

try_sed() {
    case "$1" in
        */$2/*)
            do_sed "$1"
            ;;
    esac
}

while [ $# -gt 0 ]; do
    try_sed "$1" "$COMPONENT"
    shift
done
