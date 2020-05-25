[![Snap Status](https://build.snapcraft.io/badge/anbox/anbox.svg)](https://build.snapcraft.io/user/anbox/anbox)
[![Build Status](https://travis-ci.org/anbox/anbox.svg?branch=master)](https://travis-ci.org/anbox/anbox)

# Anbox

Anbox is a container-based approach to boot a full Android system on a
regular GNU/Linux system like Ubuntu. In other words: Anbox will let
you run Android on your Linux system without the slowness of
virtualization.

## Overview

Anbox uses Linux namespaces (user, pid, uts, net, mount, ipc) to run a
full Android system in a container and provide Android applications on
any GNU/Linux-based platform.

The Android inside the container has no direct access to any hardware.
All hardware access is going through the anbox daemon on the host. We're
reusing what Android implemented within the QEMU-based emulator for OpenGL
ES accelerated rendering. The Android system inside the container uses
different pipes to communicate with the host system and sends all hardware
access commands through these.

For more details have a look at the following documentation pages:

 * [Android Hardware OpenGL ES emulation design overview](https://android.googlesource.com/platform/external/qemu/+/emu-master-dev/android/android-emugl/DESIGN)
 * [Android QEMU fast pipes](https://android.googlesource.com/platform/external/qemu/+/emu-master-dev/android/docs/ANDROID-QEMU-PIPE.TXT)
 * [The Android "qemud" multiplexing daemon](https://android.googlesource.com/platform/external/qemu/+/emu-master-dev/android/docs/ANDROID-QEMUD.TXT)
 * [Android qemud services](https://android.googlesource.com/platform/external/qemu/+/emu-master-dev/android/docs/ANDROID-QEMUD-SERVICES.TXT)

Anbox is currently suited for the desktop use case but can be used on
mobile operating systems like Ubuntu Touch, Sailfish OS or Lune OS too.
However as the mapping of Android applications is currently desktop specific
this needs additional work to supported stacked window user interfaces too.

The Android runtime environment ships with a minimal customized Android system
image based on the [Android Open Source Project](https://source.android.com/).
The used image is currently based on Android 7.1.1

## Installation

See our [installation instructions](docs/install.md) for details.

## Supported Linux Distributions

At the moment we officially support the following Linux distributions:

 * Ubuntu 16.04 (xenial)
 * Ubuntu 18.04 (bionic)

However all other distributions supporting snap packages should work as
well as long as they provide the mandatory kernel modules (see kernel/).

## Install and Run Android Applications

TBD

## Build from source

### Requirements

To build the Anbox runtime itself there is nothing special to know. We're using
cmake as build system. A few build dependencies need to be present on your host
system:

 * libdbus
 * google-mock
 * google-test
 * libboost
 * libboost-filesystem
 * libboost-log
 * libboost-iostreams
 * libboost-program-options
 * libboost-system
 * libboost-test
 * libboost-thread
 * libcap
 * libsystemd
 * mesa (libegl1, libgles2)
 * libglm
 * libsdl2
 * libprotobuf
 * protobuf-compiler
 * python2
 * lxc (>= 3.0)

On an Ubuntu system you can install all build dependencies with the following
command:

```
$ sudo apt install build-essential cmake cmake-data debhelper dbus google-mock \
    libboost-dev libboost-filesystem-dev libboost-log-dev libboost-iostreams-dev \
    libboost-program-options-dev libboost-system-dev libboost-test-dev \
    libboost-thread-dev libcap-dev libsystemd-dev libegl1-mesa-dev \
    libgles2-mesa-dev libglm-dev libgtest-dev liblxc1 \
    libproperties-cpp-dev libprotobuf-dev libsdl2-dev libsdl2-image-dev lxc-dev \
    pkg-config protobuf-compiler python-minimal
```
We recommend Ubuntu 18.04 (bionic) with **GCC 7.x** as your build environment.


### Build

Afterwards you can build Anbox with

```
$ git clone https://github.com/anbox/anbox.git
$ cd anbox
$ mkdir build
$ cd build
$ cmake ..
$ make
```

A simple

```
$ sudo make install
```

will install the necessary bits into your system.

If you want to build the anbox snap instead you can do this with the following
steps:

```
$ mkdir android-images
$ cp /path/to/android.img android-images/android.img
$ snapcraft
```

The result will be a .snap file you can install on a system supporting snaps

```
$ snap install --dangerous --devmode anbox_1_amd64.snap
```

## Run Anbox

Running Anbox from a local build requires a few more things you need to know
about. Please have a look at the ["Runtime Setup"](docs/runtime-setup.md)
documentation.

## Documentation

You will find additional documentation for Anbox in the *docs* subdirectory
of the project source.

Interesting things to have a look at

 * [Runtime Setup](docs/runtime-setup.md)
 * [Build Android image](docs/build-android.md)
 * [Generate Android emugl source](docs/generate-emugl-source.md)

## Reporting bugs

If you have found an issue with Anbox, please [file a bug](https://github.com/anbox/anbox/issues/new).

## Get in Touch

If you want to get in contact with the developers please feel free to join the
*#anbox* IRC channel on [Freenode](https://freenode.net/).

## Copyright and Licensing

Anbox reuses code from other projects like the Android QEMU emulator. These
projects are available in the external/ subdirectory with the licensing terms
included.

The Anbox source itself, if not stated differently in the relevant source files,
is licensed under the terms of the GPLv3 license.
