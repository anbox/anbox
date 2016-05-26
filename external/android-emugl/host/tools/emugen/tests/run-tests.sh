#!/bin/sh

# Copyright 2014 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -e

export LANG=C
export LC_ALL=C

PROGDIR=$(dirname "$0")
PROGNAME=$(basename "$0")

fatal () {
    echo "ERROR: $@"
    exit 1
}

OPT_EMUGEN=
OPT_HELP=
OPT_OUT_DIR=
OPT_TOOL=

for OPT; do
    OPTARG=$(expr "x$OPT" : "x[^=]*=\\(.*\\)" || true)
    case $OPT in
    --help|-h|-?)
        OPT_HELP=true
        ;;
    --emugen=*)
        OPT_EMUGEN=$OPTARG
        ;;
    --out-dir=*)
        OPT_OUT_DIR=$OPTARG
        ;;
    --tool=*)
        OPT_TOOL=$OPTARG
        ;;
    -*)
        fatal "Invalid option '$OPT', see --help."
        ;;
    *)
        fatal "This script doesn't take arguments, see --help."
        ;;
    esac
done

if [ "$OPT_HELP" ]; then
    cat <<EOF
Usage: $PROGNAME [options]

Run the emugen test suite. This scripts looks for sub-directories
named t.<number>/input, and uses them as input to 'emugen'. It then
compares the output to t.<number>/expected/ content.

Valid options:
    --help|-h|-?         Print this help.
    --out-dir=<dir>      Generate outputs into <dir>.
    --emugen=<program>   Emugen program path, if not in path.
    --tool=<tool>        Launch visual diff tool in case of differences.
EOF
    exit 0
fi

# Find emugen program
EMUGEN=
if [ "$OPT_EMUGEN" ]; then
    EMUGEN=$OPT_EMUGEN
else
    EMUGEN=$(which emugen 2>/dev/null || true)
    if [ -z "$EMUGEN" ]; then
        fatal "Cannot find 'emugen' program in PATH, use --emugen=<program> option."
    fi
    echo "Auto-config: --emugen=$EMUGEN"
fi
if [ ! -f "$EMUGEN" ]; then
    fatal "Emugen program doesn't exist: $EMUGEN"
fi

# Create output directory.
OUT_DIR=
if [ "$OPT_OUT_DIR" ]; then
    OUT_DIR=$OPT_OUT_DIR
else
    OUT_DIR=/tmp/$USER-emugen-testing
    echo "Auto-config: --out-dir=$OUT_DIR"
fi
mkdir -p "$OUT_DIR" && rm -rf "$OUT_DIR/emugen"

OUT_DIR=$OUT_DIR/emugen

# Find test directories
TEST_DIRS=$(cd "$PROGDIR" && find . -name "t.*" | sed -e 's|^\./||')
for TEST_DIR in $TEST_DIRS; do
    IN=$PROGDIR/$TEST_DIR/input
    PREFIXES=$(cd $IN && find . -name "*.in" | sed -e 's|^\./||g' -e 's|\.in$||g')
    OUT=$OUT_DIR/$TEST_DIR
    mkdir -p "$OUT/encoder"
    mkdir -p "$OUT/decoder"
    mkdir -p "$OUT/wrapper"
    for PREFIX in $PREFIXES; do
        echo "Processing $IN/foo.*"
        $EMUGEN -i "$PROGDIR/$TEST_DIR/input" -D "$OUT/decoder" -E "$OUT/encoder" -W "$OUT/wrapper" $PREFIX
    done
    if ! diff -qr "$PROGDIR/$TEST_DIR/expected" "$OUT"; then
        if [ "$OPT_TOOL" ]; then
            $OPT_TOOL "$PROGDIR/$TEST_DIR/expected" "$OUT"
        else
            echo "ERROR: Invalid differences between actual and expected output!"
            diff -burN "$PROGDIR/$TEST_DIR/expected" "$OUT"
            exit 1
        fi
    fi
done

echo "All good!"
exit 0
