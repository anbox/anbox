#!/bin/bash

set -xeuo pipefail

srcd=$(cd $(dirname $0) && pwd)
bn=$(basename $0)
tempdir=$(mktemp -d /var/tmp/tap-test.XXXXXX)
touch ${tempdir}/.testtmp
function cleanup () {
    if test -n "${TEST_SKIP_CLEANUP:-}"; then
	echo "Skipping cleanup of ${test_tmpdir}"
    else if test -f ${tempdir}/.test; then
	rm "${tempdir}" -rf
    fi
    fi
}
trap cleanup EXIT
cd ${tempdir}

assert_not_reached () {
    echo $@ 1>&2; exit 1
}

assert_file_has_content () {
    if ! grep -q -e "$2" "$1"; then
	echo 1>&2 "File '$1' doesn't match regexp '$2'"; exit 1
    fi
}

# At the moment we're testing in Travis' container infrastructure
# which also uses PR_SET_NO_NEW_PRIVS...but let's at least
# verify --help works!
bwrap --help >out.txt 2>&1
assert_file_has_content out.txt "--lock-file"

