# Install Anbox

To install Anbox your system need to support [snaps](https://snapcraft.io). We
do not officially support any other distribution method of Anbox at the moment
but there are community made packages for various distributions (e.g. Arch Linux).
However please keep in mind that the Anbox project can not give support to them
and its solely in the responsibility of the community packager to keep up with
upstream development and update the packaging to any new changes. Please feel
free to report still any bugs you encounter as they may not be related to the
packaging.

If you don't know about snaps yet head over to [snapcraft.io](https://snapcraft.io/)
to get an introduction of what snaps are, how to install support for them on your
distribution and how to use them.

The installation of Anbox consists of two steps.

 1. Getting the necessary kernel support
 2. Install the Anbox snap

## Have necessary kernel support

### On current Debian (from Buster) or Ubuntu (from 18.04)

There is no need to install those modules on an up-to-date Ubuntu or Debian:
Ubuntu and Debian have got the support compiled as modules. However, you may
get an error loading `ashmem_linux` if SecureBoot is enabled:

```
 $ sudo modprobe ashmem_linux
 modprobe: ERROR: could not insert 'ashmem_linux': Operation not permitted
```

You can confirm that SecureBoot is enabled by running the following command:

```
$ sudo mokutil --sb-state
SecureBoot enabled
```

There are two ways around this. One is to disable the SecureBoot: https://wiki.ubuntu.com/UEFI/SecureBoot/DKMS.
Following [this post](https://github.com/anbox/anbox/issues/1570), the other way is to sign the `ashmem_linux` kernel module yourself. Note that you may have to enroll your own key, as described [here](https://ubuntu.com/blog/how-to-sign-things-for-secure-boot).

### On every other Linux distribution out there

You need a kernel that's compiled with binder and ashmem support.

## Verify that kernel modules are loaded

Now you should have two new nodes in your systems `/dev` directory:

```sh
 $ ls -1 /dev/{ashmem,binder}
 /dev/ashmem
 /dev/binder
```

> In Ubuntu 19.10 the binder driver doesn't create /dev/binder when loaded. That is intentional. 
> Instead it provides support with binderfs (see https://brauner.github.io/2019/01/09/android-binderfs.html) 
> which is instead since PR anbox/anbox#1309

You can use `grep binder /proc/filesystems || echo "binderfs support is missing"`
To check for binderfs support. Note that only the snap supports binderfs
without manual intervention as of now (PR welcome).

## Install the Anbox snap

Installing the Anbox snap is very simple:

```
 $ snap install --devmode --edge anbox
```

If you haven't logged into the Ubuntu Store yet, the `snap` command will
ask you to use `sudo snap ...` in order to install the snap:

```
 $ sudo snap install --devmode --edge anbox
```

At the moment we require the use of `--devmode` as the Anbox snap is not
yet fully confined. Work has started with the upstream `snapd` project to
get support for full confinement.

As a side effect of using `--devmode` the snap will not automatically update.
In order to update to a newer version you can run:

```
 $ snap refresh --edge --devmode anbox
```

Information about the currently available versions of the snap is available
via:

```
 $ snap info anbox
```

## Available snap channels

Currently we only use the edge channels for the Anbox snap. The edge channel
tracks the latest development is always synced with the state of the master
branch on github. There is a second release channel, beta, that's getting old.

Once proper confinement for the Anbox snap exists we will also start using the
candidate and stable channels.

# Uninstall Anbox

If you want to remove Anbox from your system you first have to remove the snap:

**NOTE:** By removing the snap you remove all data you stored within the snap
from your system. There is no way to bring it back.

```
 $ snap remove anbox
```

Once the snap is removed you may have to remove the legacy DKMS modules (if you
installed them):

```
 $ sudo apt install ppa-purge
 $ sudo ppa-purge ppa:morphis/anbox-support
```

Once done Anbox is removed from your system.
