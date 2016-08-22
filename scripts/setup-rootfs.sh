#!/bin/bash

# We need to put the rootfs somewhere where we can modify some
# parts of the content on first boot (namely file permissions).
# Other than that nothing should ever modify the content of the
# rootfs.

ROOTFS_PATH=$SNAP_COMMON/var/lib/anbox/rootfs

if [ -d $ROOTFS_PATH ] ; then
	rm -rf $ROOTFS_PATH
fi

echo "Copying rootfs into $ROOTFS_PATH .."
mkdir -p $ROOTFS_PATH
tar xf $SNAP/anbox-android-rootfs.tar -C $ROOTFS_PATH/ --strip-components=1
