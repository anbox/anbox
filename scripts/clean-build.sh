#!/bin/sh

set -ex

apt-get update -qq
apt-get install -qq -y \
  build-essential \
  cmake \
  cmake-data \
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
  libdbus-1-dev \
  libdbus-cpp-dev \
  libegl1-mesa-dev \
  libgles2-mesa-dev \
  libglib2.0-dev \
  libglm-dev \
  libgtest-dev \
  liblxc1 \
  libprotobuf-dev \
  libsdl2-dev \
  lxc-dev \
  pkg-config \
  protobuf-compiler

cd /anbox

# In cases where anbox comes directly from a checked out Android
# build environment we miss some symlinks which are present on
# the host and don't have a valid git repository in that case.
git clean -fdx . || true
git reset --hard || true

mkdir build || rm -rf build/*
cd build
cmake ..
make -j10
make test
