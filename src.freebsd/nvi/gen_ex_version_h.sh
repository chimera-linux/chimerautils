#!/bin/sh
exec sed -n 's/^.*version \([^)]*)\).*/\#define VI_VERSION "\1"/p' "$@"
