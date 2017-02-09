# Anbox

Anbox is container based approach to boot a full Android system on a
regular Linux system like Ubuntu.

## Overview

Anbox uses Linux namespaces (user, pid, uts, net, mount, ipc) to run a
full Android system in a container and provide Android applications on
any platform.

Android inside the container has no direct access to any hardware. All
hardware access is going through the anbox daemon. We're reusing what
Android has implemented for the Qemu based emulator. The Android system
inside the container uses different pipes to communicate with the host
system and sends all hardware access commands through these. OpenGL
rendering is provided through this.

For more details have a look at the following documentation pages:

 * Android Hardware OpenGLES emulation design overview
   (https://goo.gl/O2Yi6x)
 * Android Qemu fast pipes (https://goo.gl/jl4GeS)
 * The Android "qemud" multiplexing daemon (https://goo.gl/DeYa5J)
 * Android Qemud services (https://goo.gl/W8Lx6t)

## Installation

As first step you need to install additional kernel drivers for the
Android binder and ashmem subsystems. Those drivers are packaged as
a DKMS package for Ubuntu 16.04 already. You can install them from
a ppa with the following commands:

```
 $ sudo apt install software-properties-common
 $ sudo add-apt-repository ppa:morphis/anbox-support
 $ sudo apt update
 $ sudo apt install anbox-modules-dkms
```

Anbox is available as a snap in the public Ubuntu Store. Currently it
is only available in the edge channel and requires to be installed in
devmode as we don't have proper confinement for it in place yet.

Anbox can be installed from the Ubuntu Store with

```
$ snap install --edge --devmode anbox
```


Afterwards run it with

```
$ anbox
```

After the first installation the container management service needs
a few minutes to setup the container the first time before it is
available.

Applications can be launched via the launch subcommand of the anbox
binary. For example

```
$ anbox launch --package com.android.settings
```

When installed as snap there will be also a desktop launcher available
which will directly start the application viewer activity to give
an overview of available Android applications and allows to start
them.

## Build from source

To build the Anbox runtime itself there is nothing special to know
about. We're using cmake as build system.

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

That will build the whole stack. A simple

```
$ make install
```

will install the necessary bits into your system.

## Copyright and Licensing

Anbox reuses code from other projects like the Android Qemu emulator.
These projects are available in the external/ subdirectory with the
licensing terms included.

The anbox source itself (in src/) is licensed under the terms of
the GPLv3 license:

Copyright (C) 2016 Simon Fels <morphis@gravedo.de>

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License version 3, as published
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranties of
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program.  If not, see <http://www.gnu.org/licenses/>.
