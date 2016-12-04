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
	chown -R root:root $ROOTFS_PATH
	echo $ROOTFS_VERSION > $ROOTFS_PATH/.version
fi

# Load binder and ashmem kernel drivers. This will just horrible break
# if kernel versions are changing ...
insmod $SNAP/binder_linux.ko || true
chmod 666 /dev/binder
insmod $SNAP/ashmem_linux.ko || true
chmod 666 /dev/ashmem

# Make sure our setup path for the container rootfs
# is present as lxc is statically configured for
# this path.
mkdir -p $SNAP_COMMON/lxc

# We start the bridge here as long as a oneshot service unit is not
# possible. See snapcraft.yaml for further details.
$SNAP/bin/anbox-bridge.sh start

$SNAP/usr/sbin/aa-exec -p unconfined -- $SNAP/bin/anbox-wrapper.sh container-manager
pid=$!
waitpid $pid

$SNAP/bin/anbox-bridge.sh stop
