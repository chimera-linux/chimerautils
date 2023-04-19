#!/bin/sh

ERRTYPE=$1
ERRLIST=$2
ERRTYPE_U=$(echo $ERRTYPE|tr '[:lower:]' '[:upper:]')

echo "static struct fetcherr ${ERRTYPE}_errlist[] = {"
cat "$ERRLIST" | grep -v "^#" | sort | while read NUM CAT STRING; do
    echo "    {${NUM}, FETCH_${CAT}, \"${STRING}\"},"
done
echo "    {-1, FETCH_UNKNOWN, \"Unknown ${ERRTYPE_U} error\"}"
echo "};"
