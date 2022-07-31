#!/bin/sh
sed -e "s,@BINDIR@,$3,g" -e "s,@LIBEXECDIR@,$4,g" "$1" > "$2"
