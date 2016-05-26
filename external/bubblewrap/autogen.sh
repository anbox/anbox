#!/bin/sh

test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.

olddir=`pwd`
cd $srcdir

if ! (autoreconf --version >/dev/null 2>&1); then
        echo "*** No autoreconf found, please install it ***"
        exit 1
fi

mkdir -p m4

autoreconf --force --install --verbose

cd $olddir
test -n "$NOCONFIGURE" || "$srcdir/configure" "$@"
