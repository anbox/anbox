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

ANBOX_DIR="$PWD"

OPT_ANBOX_DIR=
OPT_EMUGEN=
OPT_HELP=

for OPT; do
    OPTARG=$(expr "x$OPT" : "x[^=]*=\\(.*\\)" || true)
    case $OPT in
    --help|-h|-\?)
        OPT_HELP=true
        ;;
    --anbox-dir=*)
        OPT_ANBOX_DIR=$OPTARG
        ;;
    --emugen=*)
        OPT_EMUGEN=$OPTARG
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

Update the sources of the GPU emulation encoder and decoder, after an update
to the 'emugen' tool.

Valid options (defaults are in brackets):
    --help|-h|-?         Print this help.
    --anbox-dir=<dir>    Specify top-level Anbox directory [$ANBOX_DIR].
    --emugen=<program>   Emugen program path, if not in path.
EOF
    exit 0
fi

EMUGEN=
if [ "$OPT_EMUGEN" ]; then
    EMUGEN=$OPT_EMUGEN
    if [ ! -f "$EMUGEN" ]; then
        fatal "Missing emugen binary: $EMUGEN"
    fi
else
    EMUGEN=$PROGDIR/../../../objs/build/intermediates64/emugen/emugen
    if [ ! -f "$EMUGEN" ]; then
        fatal "Missing emugen binary: $EMUGEN, please build it or use --emugen=<program>"
    fi
    echo "Auto-config: --emugen=$EMUGEN"
fi

if [ "$OPT_ANBOX_DIR" ]; then
    ANBOX_DIR=$OPT_ANBOX_DIR
else
    echo "Auto-config: --anbox-dir=$ANBOX_DIR"
fi
if [ ! -d "$ANBOX_DIR/src/anbox" ]; then
    fatal "Not an Anbox directory: $ANBOX_DIR"
fi

ANBOX_DIR=$(cd "$ANBOX_DIR" && pwd -P)

# Find the sources for the encoder:
ENCODER_TOP_DIR=$ANBOX_DIR/android/opengl/system
DECODER_TOP_DIR=$ANBOX_DIR/external/android-emugl/host/libs

if [ ! -d "$ENCODER_TOP_DIR" ]; then
    fatal "Missing encoder source directory: $ENCODER_TOP_DIR"
fi
if [ ! -d "$DECODER_TOP_DIR" ]; then
    fatal "Missing decoder source directory: $DECODER_TOP_DIR"
fi

# GLESv1 encoder
GLESv1_INPUT_DIR=$DECODER_TOP_DIR/GLESv1_dec
GLESv2_INPUT_DIR=$DECODER_TOP_DIR/GLESv2_dec
RENDERCONTROL_INPUT_DIR=$DECODER_TOP_DIR/renderControl_dec

# The encoder has prefix GL while decoder has GLES
cp -f $DECODER_TOP_DIR/GLESv1_dec/gles1.attrib $DECODER_TOP_DIR/GLESv1_dec/gl.attrib
cp -f $DECODER_TOP_DIR/GLESv1_dec/gles1.in $DECODER_TOP_DIR/GLESv1_dec/gl.in
cp -f $DECODER_TOP_DIR/GLESv1_dec/gles1.types $DECODER_TOP_DIR/GLESv1_dec/gl.types

cp -f $DECODER_TOP_DIR/GLESv2_dec/gles2.attrib $DECODER_TOP_DIR/GLESv2_dec/gl2.attrib
cp -f $DECODER_TOP_DIR/GLESv2_dec/gles2.in $DECODER_TOP_DIR/GLESv2_dec/gl2.in
cp -f $DECODER_TOP_DIR/GLESv2_dec/gles2.types $DECODER_TOP_DIR/GLESv2_dec/gl2.types

$EMUGEN -i $DECODER_TOP_DIR/GLESv1_dec -E $ENCODER_TOP_DIR/GLESv1_enc gl
$EMUGEN -i $DECODER_TOP_DIR/GLESv2_dec -E $ENCODER_TOP_DIR/GLESv2_enc gl2
$EMUGEN -i $DECODER_TOP_DIR/renderControl_dec -E $ENCODER_TOP_DIR/renderControl_enc renderControl

rm $DECODER_TOP_DIR/GLESv1_dec/gl.attrib
rm $DECODER_TOP_DIR/GLESv1_dec/gl.in
rm $DECODER_TOP_DIR/GLESv1_dec/gl.types

rm $DECODER_TOP_DIR/GLESv2_dec/gl2.attrib
rm $DECODER_TOP_DIR/GLESv2_dec/gl2.in
rm $DECODER_TOP_DIR/GLESv2_dec/gl2.types

echo "Done, see $ENCODER_TOP_DIR"
exit 0
