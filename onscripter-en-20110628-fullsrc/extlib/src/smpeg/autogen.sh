#!/bin/sh
#

die() {
	echo "'$0' failed to run properly"
	exit 1
}

set -e

aclocal $ACLOCAL_FLAGS || die aclocal
automake --foreign || die automake
autoconf || die autoconf

#./configure $*
echo "Now you are ready to run ./configure"
