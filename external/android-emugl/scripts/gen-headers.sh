#!/bin/sh

# Copyright (C) 2015 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Run this script to re-generate headers with the gen-entries.py script.

set -e
export LANG=C
export LC_ALL=C

PROGDIR=$(dirname "$0")

panic () {
    echo "ERROR: $@"
    exit 1
}

QEMU_TOP_DIR=$(cd $PROGDIR/../../.. && pwd -P)
SCRIPT_DIR=android/scripts
if [ ! -d "$QEMU_TOP_DIR/$SCRIPT_DIR" ]; then
    panic "Missing scripts directory: $QEMU_TOP_DIR/$SCRIPT_DIR"
fi

cd $QEMU_TOP_DIR
GEN_ENTRIES=$SCRIPT_DIR/gen-entries.py
if [ ! -f "$GEN_ENTRIES" ]; then
    panic "Missing script: $GEN_ENTRIES"
fi

FAILURES=

# $1: Source file
# $2: Target file
# $3: gen-entries script.
gen_functions_header () {
    local SRC_FILE="$1"
    local DST_FILE="$2"
    local GEN_ENTRIES="$3"
    if [ ! -f "$SRC_FILE" ]; then
        echo "ERROR: Missing source file: $SRC_FILE"
        FAILURES=true
    else
        echo "Generating $DST_FILE"
        $GEN_ENTRIES --mode=functions $SRC_FILE --output=$DST_FILE
    fi
}

# $1: Source file
# $2: Target file
# $3: gen-entries script.
gen_funcargs_header () {
    local SRC_FILE="$1"
    local DST_FILE="$2"
    local GEN_ENTRIES="$3"
    if [ ! -f "$SRC_FILE" ]; then
        echo "ERROR: Missing source file: $SRC_FILE"
        FAILURES=true
    else
        echo "Generating $DST_FILE"
        $GEN_ENTRIES --mode=funcargs $SRC_FILE --output=$DST_FILE
    fi
}


##
##  libOpenglRender headers.
##
LIBRENDER_DIR=distrib/android-emugl/host/libs/libOpenglRender
LIBRENDER_INCLUDE=distrib/android-emugl/host/include/OpenglRender
gen_funcargs_header \
        "$LIBRENDER_DIR"/render_api.entries \
        "$LIBRENDER_INCLUDE"/render_api_functions.h \
        "$GEN_ENTRIES"

##
## libOpenGLESDispatch headers.
##
OPENGLES_DISPATCH_SRCDIR=distrib/android-emugl/host/libs/libOpenGLESDispatch
OPENGLES_DISPATCH_INCLUDE=distrib/android-emugl/host/include/OpenGLESDispatch

gen_functions_header \
        "$OPENGLES_DISPATCH_SRCDIR"/render_egl.entries \
        "$OPENGLES_DISPATCH_INCLUDE"/RenderEGL_functions.h \
        "$GEN_ENTRIES"

gen_functions_header \
        "$OPENGLES_DISPATCH_SRCDIR"/render_egl_extensions.entries \
        "$OPENGLES_DISPATCH_INCLUDE"/RenderEGL_extensions_functions.h \
        "$GEN_ENTRIES"

GLES_ENTRIES="gles_common gles_extensions gles1_only gles1_extensions gles2_only \
gles2_extensions gles3_only"

for ENTRY in $GLES_ENTRIES; do
    SRC_FILE=$OPENGLES_DISPATCH_SRCDIR/${ENTRY}.entries
    DST_FILE=$OPENGLES_DISPATCH_INCLUDE/${ENTRY}_functions.h
    gen_funcargs_header "$SRC_FILE" "$DST_FILE" "$GEN_ENTRIES"
done

if [ "$FAILURES" ]; then
    exit 1
fi
