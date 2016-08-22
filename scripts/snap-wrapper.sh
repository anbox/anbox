#!/bin/bash

if [ "$SNAP_ARCH" == "amd64" ]; then
	ARCH="x86_64-linux-gnu"
elif [ "$SNAP_ARCH" == "armhf" ]; then
	ARCH="arm-linux-gnueabihf"
else
	ARCH="$SNAP_ARCH-linux-gnu"
fi

export LD_LIBRARY_PATH=$SNAP/usr/lib/$ARCH:$LD_LIBRARY_PATH

# Mesa Libs
export LD_LIBRARY_PATH=$SNAP/usr/lib/$ARCH/mesa:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$SNAP/usr/lib/$ARCH/mesa-egl:$LD_LIBRARY_PATH

# XDG Config
export XDG_CONFIG_DIRS=$SNAP/etc/xdg:$XDG_CONFIG_DIRS
export XDG_CONFIG_DIRS=$SNAP/usr/xdg:$XDG_CONFIG_DIRS
# Note: this doesn't seem to work, QML's LocalStorage either ignores
# or fails to use $SNAP_USER_DATA if defined here
export XDG_DATA_DIRS=$SNAP_USER_DATA:$XDG_DATA_DIRS
export XDG_DATA_DIRS=$SNAP/usr/share:$XDG_DATA_DIRS

# Not good, needed for fontconfig
export XDG_DATA_HOME=$SNAP/usr/share

# Tell libGL where to find the drivers
export LIBGL_DRIVERS_PATH=$SNAP/usr/lib/$ARCH/dri

# ensure the snappy gl libs win
export LD_LIBRARY_PATH="$SNAP_LIBRARY_PATH:$LD_LIBRARY_PATH"

cd $SNAP

if [ "$(id -u)" == "0" ] ; then
	# Make sure our setup path for the container rootfs
	# is present as lxc is statically configured for
	# this path.
	mkdir -p $SNAP_COMMON/lxc
fi

exec $SNAP/usr/bin/anbox $@
