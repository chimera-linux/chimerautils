#!/bin/sh

do_sed() {
    sed -n 's/^ \* PUBLIC: \(.*\)/\1/p' "$@"
}

try_sed() {
    case "$1" in
        */$2/*)
            do_sed "$1"
            return 0
            ;;
    esac
    return 1
}

echo "#ifdef CL_IN_EX"

while try_sed "$1" cl; do
    shift
done

echo "#endif"
echo "#ifdef EXP"

while try_sed "$1" ex; do
    shift
done

echo "#endif"
echo "#ifdef V_ABS"

while try_sed "$1" vi; do
    shift
done

echo "#endif"

do_sed "$@"
