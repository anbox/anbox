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

 1. Install necessary kernel modules
 2. Install the Anbox snap

In order to support the mandatory kernel subsystems ashmem and binder for the
Android container you have to install two
[DKMS](https://en.wikipedia.org/wiki/Dynamic_Kernel_Module_Support)
based kernel modules. The source for the kernel modules is maintained by the
Anbox project [here](https://github.com/anbox/anbox-modules).

At the moment we only have packages prepared for Ubuntu in a PPA on Launchpad.
If you want to help to get the packages in your favorite distribution please
come and talk to us or submit a PR with the distribution specific packaging.

The second step will install the Anbox snap from the store and will give you
everything you need to run the full Anbox experience.

## Install necessary kernel modules

### On Ubuntu 19.04 and later

There is no need to install those modules on a recent Ubuntu or Debian: Even Ubuntu 18.04 LTS and Debian Stable have got the modules. However, you may get an error loading `ashmem_linux` if SecureBoot is enabled:

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

### Before Ubuntu 19.04

You should have the modules on your system already. However, you can still try to use the ppa (no longer maintained) if `sudo modprobe ashmem_linux binder_linux` can't find the modules.

In order to add the PPA to your Ubuntu system please run the following commands:

```
 $ sudo add-apt-repository ppa:morphis/anbox-support
 $ sudo apt update
 $ sudo apt install anbox-modules-dkms
```
> In case `add-apt-repository` is missing, install it via:
> ```
> sudo apt install software-properties-common
> ```

These will add the PPA to your system and install the `anbox-modules-dkms`
package which contains the ashmem and binder kernel modules. They will be
automatically rebuild everytime the kernel packages on your system update.

> On system with UEFI the Secure Boot should be disabled, see
> https://wiki.ubuntu.com/UEFI/SecureBoot/DKMS

After you installed the `anbox-modules-dkms` package you have to manually
load the kernel modules. The next time your system starts they will be
automatically loaded.

```
 $ sudo modprobe ashmem_linux
 $ sudo modprobe binder_linux
```

## Verify that kernel modules are loaded

Now you should have two new nodes in your systems `/dev` directory:

```
 $ ls -1 /dev/{ashmem,binder}
 /dev/ashmem
 /dev/binder
```

> In Ubuntu 19.10 the binder driver doesn't create /dev/binder when loaded. That is intentional. 
> Instead it provides support for binderfs (see https://brauner.github.io/2019/01/09/android-binderfs.html) 
> which is instead since PR anbox/anbox#1309

## Install the Anbox snap

Installing the Anbox snap is very simple:

```
 $ snap install --devmode --beta anbox
```

If you haven't logged into the Ubuntu Store yet, the `snap` command will
ask you to use `sudo snap ...` in order to install the snap:

```
 $ sudo snap install --devmode --beta anbox
```

At the moment we require the use of `--devmode` as the Anbox snap is not
yet fully confined. Work has started with the upstream `snapd` project to
get support for full confinement.

As a side effect of using `--devmode` the snap will not automatically update.
In order to update to a newer version you can run:

```
 $ snap refresh --beta --devmode anbox
```

Information about the currently available versions of the snap is available
via:

```
 $ snap info anbox
```

## Available snap channels

Currently we only use the beta and edge channels for the Anbox snap. The edge
channel tracks the latest development is always synced with the state of the
master branch on github. The beta channel is updated less frequently to provide
a more stable and bug free experience.

Once proper confinement for the Anbox snap exists we will also start using the
candidate and stable channels.

# Uninstall Anbox

If you want to remove Anbox from your system you first have to remove the snap:

**NOTE:** By removing the snap you remove all data you stored within the snap
from your system. There is no way to bring it back.

```
 $ snap remove anbox
```

Once the snap is removed you have to remove the installed kernel modules as well:

```
 $ sudo apt install ppa-purge
 $ sudo ppa-purge ppa:morphis/anbox-support
```

Once done Anbox is removed from your system.
