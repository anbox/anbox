#!/bin/bash

# We need to put the rootfs somewhere where we can modify some
# parts of the content on first boot (namely file permissions).
# Other than that nothing should ever modify the content of the
# rootfs.

DATA_PATH=$SNAP_COMMON/var/lib/anbox
ROOTFS_PATH=$DATA_PATH/rootfs
RAMDISK_PATH=$DATA_PATH/ramdisk
INITRD=$SNAP/ramdisk.img
SYSTEM_IMG=$SNAP/system.img
ANDROID_DATA_PATH=$DATA_PATH/android-data

if [ ! -e $INITRD ]; then
	echo "ERROR: boot ramdisk does not exist"
	exit 1
fi

if [ ! -e $SYSTEM_IMG ]; then
	echo "ERROR: system image does not exist"
	exit 1
fi

# Extract ramdisk content instead of trying to bind mount the
# cpio image file to allow modifications.
rm -Rf $RAMDISK_PATH
mkdir -p $RAMDISK_PATH
cd $RAMDISK_PATH
cat $INITRD | gzip -d | cpio -i

# FIXME those things should be fixed in the build process
chmod +x $RAMDISK_PATH/anbox-init.sh

# Setup the read-only rootfs
mkdir -p $ROOTFS_PATH
mount -o bind,ro $RAMDISK_PATH $ROOTFS_PATH
mount -o loop,ro $SYSTEM_IMG $ROOTFS_PATH/system

# ... but we keep /data in the read/write space
mkdir -p $ANDROID_DATA_PATH
mount -o bind $ANDROID_DATA_PATH $ROOTFS_PATH/data

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

umount $ROOTFS_PATH/system
umount $ROOTFS_PATH/data
