[![Build Status](https://travis-ci.org/anbox/anbox.svg?branch=master)](https://travis-ci.org/anbox/anbox)

Anbox is a container-based approach to boot a virtual Android 7.1.1 (nougat) device on a regular GNU/Linux operating system. We use [LXC](https://linuxcontainers.org/) to run a modified [Android Open Source Project](https://source.android.com/) without the overhead of a hypervisor virtualization. 

In other words: Anbox will let you seamlessly run your favourite Android apps on your Linux computer.

____
&nbsp;
## **Installation**
We are distributing Anbox with [snap](https://snapcraft.io) packages:
```sh
$ sudo snap install --devmode --beta anbox
```
At the moment we require the use of `--devmode` as the Anbox snap is not
yet fully confined. Work has started with the upstream `snapd` project to
get support for full confinement. As a side effect of using `--devmode` the snap will not automatically update.

We officially support **Ubuntu 18.04 (bionic)** and **Ubuntu 20.04 (focal)** as they include everything you need to run Anbox, including the kernel modules.

&nbsp;
### **Kernel Modules**
Anbox requires [ashmem](https://elinux.org/Android_Kernel_Features#ashmem) and [binder](https://elinux.org/Android_Binder) for the Android subsystem. You can test wether they are available by executing:
```sh
$ sudo modprobe binder_linux ashmem_linux
```
Ubuntu includes them since version 18.04.4. If your Kernel does not have the modules included for whatever reason, you can use our unmaintained [Kernel Modules](https://github.com/anbox/anbox-modules).


#### **Secure Boot can cause errors when trying to load `ashmem_linux`**. There are two ways around this: 
* One is to disable the SecureBoot: https://wiki.ubuntu.com/UEFI/SecureBoot/DKMS. 
* Following [this post](https://github.com/anbox/anbox/issues/1570), the other way is to sign the `ashmem_linux` kernel module yourself. Note that you may have to enroll your own key, as described [here](https://ubuntu.com/blog/how-to-sign-things-for-secure-boot).
<!-- TODO: mode detailed desctiption of process
  - create separate guide
-->

**Additional Resources**: [Build from Source](docs/build.md)

<!-- TODO: specify feature/limitations here
  Provide a set of features that Anbox/Android can use, for example GPS mocking could be interesting for many people. On the other hand we should mention limitations, like the inability of making phone calls, just for the sake of transparancy.
-->
____
&nbsp;
## **Get in Touch**
If you want to get in contact with the developers please feel free to join the
*#anbox* IRC channel on [Freenode](https://freenode.net/) or join our [Telegram Chat](https://t.me/anbox).


## **Copyright and Licensing**
Anbox reuses code from other projects like the Android QEMU emulator. These
projects are available in the `external/` subdirectory with the licensing terms
included.

The Anbox source itself, if not stated differently in the relevant source files,
is licensed under the terms of the GPLv3 license.