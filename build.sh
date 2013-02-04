#!/bin/sh
if [ "x$CC" = "x" ]; then
    export CC="cc"
fi

if [ "x$CFLAGS" = "x" ]; then
    export CFLAGS="-Os -pedantic -pipe -std=c99 -Wall"
fi

run() {
    echo " * Running \`$*'..."
    $* || exit 1
}

run $CC $CFLAGS -o deldir deldir.c
run strip -s deldir

