#!/bin/bash

# We need to put the rootfs somewhere where we can modify some
# parts of the content on first boot (namely file permissions).
# Other than that nothing should ever modify the content of the
# rootfs.

ROOTFS_PATH=$SNAP_COMMON/var/lib/anbox/rootfs
ROOTFS_VERSION=1

if [ ! -e $ROOTFS_PATH ] || [ "$ROOTFS_VERSION" != "$(cat $ROOTFS_PATH/.version)" ] ; then
	rm -rf $ROOTFS_PATH
	echo "Copying rootfs into $ROOTFS_PATH .."
	mkdir -p $ROOTFS_PATH
	tar xf $SNAP/android-rootfs.tar -C $ROOTFS_PATH/ --strip-components=1
fi

# Make sure our setup path for the container rootfs
# is present as lxc is statically configured for
# this path.
mkdir -p $SNAP_COMMON/lxc

exec $SNAP/bin/anbox.wrapper container-manager
