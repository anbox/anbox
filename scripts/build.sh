#!/bin/sh

set -ex

# optional clean up
if [ "$1" = "--clean" ] ; then
  # In cases where anbox comes directly from a checked out Android
  # build environment we miss some symlinks which are present on
  # the host and don't have a valid git repository in that case.
  if [ -d .git ] ; then
    git clean -fdx .
    git reset --hard
    git submodule init
    git submodule update
  fi
fi

mkdir build || rm -rf build/*
cd build
cmake ..
VERBOSE=1 make -j10
VERBOSE=1 make test
cd ..
