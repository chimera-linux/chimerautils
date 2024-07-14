#!/bin/sh
#
# Updates the system database for locate(1).
#
# Based on FreeBSD's periodic script, made
# standalone by q66 <q66@chimera-linux.org>.

echo "Rebuilding locate database..."

. /etc/locate.rc
: ${FCODES:="/var/db/locate.database"}
locdb="$FCODES"

touch "$locdb" && rc=0 || rc=3
chown nobody "$locdb" || rc=3
chmod 644 "$locdb" || rc=3

cd /
echo /usr/libexec/locate.updatedb | nice -n 5 su -m nobody || rc=3
chmod 444 $locdb || rc=3

exit $rc
