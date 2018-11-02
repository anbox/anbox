#!/bin/sh

set -ex

apt-get update -qq
apt-get install -qq -y \
  build-essential \
  cmake \
  cmake-data \
  cmake-extras \
  debhelper \
  dbus \
  git \
  google-mock \
  libboost-dev \
  libboost-filesystem-dev \
  libboost-log-dev \
  libboost-iostreams-dev \
  libboost-program-options-dev \
  libboost-system-dev \
  libboost-test-dev \
  libboost-thread-dev \
  libcap-dev \
  libegl1-mesa-dev \
  libgles2-mesa-dev \
  libglm-dev \
  libgtest-dev \
  liblxc1 \
  libproperties-cpp-dev \
  libprotobuf-dev \
  libsdl2-dev \
  libsdl2-image-dev \
  libsystemd-dev \
  lxc-dev \
  pkg-config \
  protobuf-compiler

apt-get clean

cd /anbox

cleanup() {
  # In cases where anbox comes directly from a checked out Android
  # build environment we miss some symlinks which are present on
  # the host and don't have a valid git repository in that case.
  if [ -d .git ] ; then
    git clean -fdx .
    git reset --hard
  fi
}

cleanup

mkdir build || rm -rf build/*
cd build
cmake ..
VERBOSE=1 make -j10
VERBOSE=1 make test
cd ..
