#!/bin/sh

usage() {
    echo "usage: $1 [-h|--help]"
}

if [ "$1" = "-h" -o "$1" = "--help" ]; then
    usage $0
    exit 0
elif [ "$#" -gt 0 ]; then
    >&2 echo "$0: unrecognized option: $1"
    >&2 usage $0
    exit 1
fi

exec uname -m
