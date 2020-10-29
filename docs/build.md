# Compile and Run Anbox
We use `cmake` and `make` to build Anbox. We recommend Ubuntu 20.04 (focal) as your build environment.
You can build the project using a docker container as well. In that case you have to install Anbox manually.

____
&nbsp;
## **Build requirements**
To build the Anbox runtime itself there is nothing special to know. We're using
cmake as build system. A few build dependencies need to be present on your host
system:
`libdbus`, 
`google-mock`, 
`google-test`, 
`libboost`, 
`libboost-filesystem`, 
`libboost-log`, 
`libboost-iostreams`, 
`libboost-program-options`, 
`libboost-system`, 
`libboost-test`, 
`libboost-thread`, 
`libcap`, 
`libexpat1-dev`, 
`libsystemd`, 
`libegl1`, 
`libgles2`, 
`libglm`, 
`libsdl2`, 
`libprotobuf`, 
`protobuf-compiler`, 
`python2`, 
`lxc`

On Debain bases systems you can install all build dependencies with the following
command:
```
$ sudo apt install build-essential cmake cmake-data debhelper dbus google-mock libboost-dev libboost-filesystem-dev libboost-log-dev libboost-iostreams-dev libboost-program-options-dev libboost-system-dev libboost-test-dev libboost-thread-dev libcap-dev libexpat1-dev libsystemd-dev libegl1-mesa-dev libgles2-mesa-dev libglm-dev libgtest-dev liblxc1 libproperties-cpp-dev libprotobuf-dev libsdl2-dev libsdl2-image-dev lxc-dev pkg-config protobuf-compiler python-minimal
```
____
&nbsp;
## **Build Anbox Locally**
Starting out in the repositories root directory:
```sh
$ mkdir build && cd build
$ cmake .. && make -j 8

# install anbox
$ sudo make install
```

____
&nbsp;
## **Run Anbox**
To run the compiled binaries, you will need an Android image.
You can build one yourself with [this guide](docs/build-android.md) or download a precompiled image: [build.anbox.io/android-images](https://build.anbox.io/android-images/).
```sh
# download latest prebuilt android image
$ wget build.anbox.io/android-images/2018/07/19/android_amd64.img \
  --output-document=${PWD}/android_amd64.img
```

&nbsp;
### Unprivileged Containers
The following scripts will start the `container-manager` in **unprivileged** mode. 
This means, that user and group ids are different on the host and inside the container namespace. This behaviour is restricted by default and needs some modifications on your host system. We need to allow your current user to impersonate other user ids.

Assuming that your current user id is `1000`, add the following lines to `/etc/subuid` and `/etc/subgid`:
```
root:100000:999
root:1000:1
root:101001:99999
```

&nbsp;
### Using the Anbox Commandline Interface

Now you can load the kernel modules, start the Anbox container and boot Android.
```sh
#!/bin/bash

# load kernel modules
sudo modprobe ashmem_linux
sudo modprobe binder_linux

if [ ! -d "/dev/binderfs" ] ; then
  # mount binderfs 
  sudo mkdir /dev/binderfs
  sudo mount -t binder binder /dev/binderfs
fi

sudo anbox container-manager \
    --daemon \
    --android-image=${PWD}/android_amd64.img \
    --data-path=${PWD}/anbox-data
```
and
```sh
anbox session-manager
```

Finally you can launch the application overview with:
```sh
anbox launch \
  --package=org.anbox.appmgr \
    --component=org.anbox.appmgr.AppViewActivity
```
Starting the container and booting Android might take a few seconds depending on your system. During that thime the launch command might be unresponsive.

**Additional Resources**: [Command Line Interface](cli.md)

____
&nbsp;
## **Build Anbox using Docker**
The docker image is based on Ubuntu 20.04 focal and comes with all necessary dependencies.
```sh
# start container and build anbox
scripts/build-with-docker.sh
```


____
&nbsp;
## **Build Anbox snap-package**
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




















## Build from source

To build the Anbox runtime itself there is nothing special to know. We're using
cmake as build system. A few build dependencies need to be present on your host
system:
`libdbus`, 
`google-mock`, 
`google-test`, 
`libboost`, 
`libboost-filesystem`, 
`libboost-log`, 
`libboost-iostreams`, 
`libboost-program-options`, 
`libboost-system`, 
`libboost-test`, 
`libboost-thread`, 
`libcap`, 
`libexpat1-dev`, 
`libsystemd`, 
`mesa (libegl1, libgles2)`, 
`libglm`, 
`libsdl2`, 
`libprotobuf`, 
`protobuf-compiler`, 
`python2`, 
`lxc`

On an Ubuntu system you can install all build dependencies with the following
command:

```
$ sudo apt install build-essential cmake cmake-data debhelper dbus google-mock \
    libboost-dev libboost-filesystem-dev libboost-log-dev libboost-iostreams-dev \
    libboost-program-options-dev libboost-system-dev libboost-test-dev \
    libboost-thread-dev libcap-dev libexpat1-dev libsystemd-dev libegl1-mesa-dev \
    libgles2-mesa-dev libglm-dev libgtest-dev liblxc1 \
    libproperties-cpp-dev libprotobuf-dev libsdl2-dev libsdl2-image-dev lxc-dev \
    pkg-config protobuf-compiler python-minimal
```
We recommend Ubuntu 20.04 (focal) as your build environment.
