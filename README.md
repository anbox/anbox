# Anbox

Anbox is container based approach to boot a full Android system.

## Overview

Anbox uses Linux namespaces (user, pid, uts, net, mount, ipc) to run a
full Android system in a container and provide Android applications on
any platform.

The container itself does not run as root but as a unprivileged user.
It just need to be able to perform a few operations to setup a
container like spawning up new namespaces, chroot'ing to a rootfs etc.

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

## Copyright and Licensing

Anbox reuses code from other projects like the Android Qemu emulator
or bubblewrap (https://github.com/projectatomic/bubblewrap). These
projects are available in the external/ subdirectory with the
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
