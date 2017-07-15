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

The installation process currently consists of a few steps which will
add additional components to your host system. These include

 * Out-of-tree kernel modules for binder and ashmem as no distribution kernel
   ships both enabled.
 * A udev rule to set correct permissions for /dev/binder and /dev/ashmem
 * A upstart job which starts the Anbox session manager as part of
   a user session.

To make this process as easy as possible we have bundled the necessary
steps in a snap (see https://snapcraft.io) called "anbox-installer". The
installer will perform all necessary steps. You can install it on a system
providing support for snaps by running

```
$ snap install --classic anbox-installer
```

Alternatively you can fetch the installer script via

```
$ wget https://raw.githubusercontent.com/anbox/anbox-installer/master/installer.sh -O anbox-installer
```

Please note that we don't support any possible Linux distribution out there
yet. Please have a look at the following chapter to see a list of supported
distributions.

To proceed the installation process simply called

```
$ anbox-installer
```

This will guide you through the installation process.

**NOTE:** Anbox is currently in a **pre-alpha development state**. Don't expect a
fully working system for a production system with all features you need. You will
for sure see bugs and crashes. If you do so, please don't hestitate and report them!

**NOTE:** The Anbox snap currently comes **completely unconfined** and is because of
this only available from the edge channel. Proper confinement is a thing we want
to achieve in the future but due to the nature and complexity of Anbox this isn't
a simple task.

## Supported Linux Distributions

At the moment we officially support the following Linux distributions:

 * Ubuntu 16.04 (xenial)

Untested but likely to work:

 * Ubuntu 14.04 (trusty)
 * Ubuntu 16.10 (yakkety)
 * Ubuntu 17.04 (zesty)

## Install and Run Android Applications

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
 * libdbus-cpp
 * mesa (libegl1, libgles2)
 * glib-2.0
 * libsdl2
 * libprotobuf
 * protobuf-compiler
 * lxc

On an Ubuntu system you can install all build dependencies with the following
command:

```
$ sudo apt install build-essential cmake cmake-data debhelper dbus google-mock \
    libboost-dev libboost-filesystem-dev libboost-log-dev libboost-iostreams-dev \
    libboost-program-options-dev libboost-system-dev libboost-test-dev \
    libboost-thread-dev libcap-dev libdbus-1-dev libdbus-cpp-dev libegl1-mesa-dev \
    libgles2-mesa-dev libglib2.0-dev libglm-dev libgtest-dev liblxc1 \
    libproperties-cpp-dev libprotobuf-dev libsdl2-dev libsdl2-image-dev lxc-dev \
    pkg-config protobuf-compiler 
```
We recommend Ubuntu 16.04 (xenial) with **GCC 5.x** as your build environment.


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
