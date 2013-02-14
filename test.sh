#!/bin/sh

_mktemp() {
    echo -n " * Creating temporary folder... "
    export DESTDIR=`mktemp --directory --tmpdir deldir-test.XXXXXXXXXX` || export mktemp_failed="yes"
    if [ "$mktemp_failed" = "yes" ]; then
        export test_error="Could not create temporary folder"
        echo "[FAIL]"
        return 1
    else
        echo "[ ok ] (${DESTDIR})"
        return 0
    fi
}

_build() {
    if [ "${SKIP_BUILD}" = "yes" ]; then
        return 0
    fi

    echo -n " * Building deldir...           "
    make deldir >/dev/null 2>/dev/null || export build_failed="yes"
    if [ "$build_failed" = "yes" ]; then
        export test_error="Error building deldir"
        echo "[FAIL]"
        # Run make again to show the compiler error(s)
        make deldir >/dev/null
        return 1
    else
        echo "[ ok ]"
        return 0
    fi
}

_mktree() {
    echo -n " * Building directory tree...   "
    touch ${DESTDIR}/file1 || export mktree_failed="yes"
    mkdir -p ${DESTDIR}/dir1/dir2 || export mktree_failed="yes"
    touch ${DESTDIR}/dir1/file2 || export mktree_failed="yes"
    touch ${DESTDIR}/dir1/dir2/file3 || export mktree_failed="yes"
    ln -s ${DESTDIR}/file1 ${DESTDIR}/dir1/link1 || export mktree_failed="yes"
    if [ "$mktree_failed" = "yes" ]; then
        export test_error="Could not build directory tree"
        echo "[FAIL]"
        return 1
    else
        echo "[ ok ]"
        return 0
    fi
}

_deldir() {
    echo " * Running deldir...               "
    ${DELDIR_BIN} -y ${DESTDIR} || export deldir_failed="yes"
    echo -n " * Running deldir...            "
    if [ "$deldir_failed" = "yes" ]; then
        export test_error="Error running deldir"
        echo "[FAIL]"
        return 1
    else
        echo "[ ok ]"
        return 0
    fi
}

_print_remaining() {
    if [ -d ${DESTDIR} ]; then
        export test_error="Not all files/folders were deleted"
        echo
        echo "*** Remaining files: ***"
        echo
        ls -R ${DESTDIR}
        echo
        echo "*** End of remaining files***"
        echo
    fi
}


_clean_up() {
    if [ -d ${DESTDIR} ]; then
        echo -n " * Cleaning up...               "
        rm -fr ${DESTDIR} || export rm_failed="yes"
        if [ "$rm_failed" = "yes" ]; then
            echo "[FAIL]"
            return 1
        else
            echo "[ ok ]"
            return 0
        fi
    fi
}

_print_results() {
    echo
    echo -n "Self-test "
    if [ "x${test_error}" = "x" ]; then
        echo "PASSED"
    else
        echo "FAILED (${test_error})"
    fi
}

if [ "x$1" = "x" ]; then
    export DELDIR_BIN="./deldir"
else
    export DELDIR_BIN="$1"
    export SKIP_BUILD="yes"
fi

_mktemp && _build && _mktree && _deldir && _print_remaining
_clean_up ; _print_results

