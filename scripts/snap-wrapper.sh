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

# Tell libGL where to find the drivers
export LIBGL_DRIVERS_PATH=$SNAP/usr/lib/$ARCH/dri

# ensure the snappy gl libs win
export LD_LIBRARY_PATH="$SNAP_LIBRARY_PATH:$LD_LIBRARY_PATH"

# We set XDG_DATA_HOME to SNAP_USER_COMMON here as this will be the location we will
# create all our application launchers in. The system application launcher will
# be configured by our installer to look into this directory for available
# launchers.
export XDG_DATA_HOME="$SNAP_USER_COMMON"

exec $SNAP/usr/bin/anbox $@
