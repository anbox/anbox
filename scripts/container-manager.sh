#!/bin/sh

# We need to put the rootfs somewhere where we can modify some
# parts of the content on first boot (namely file permissions).
# Other than that nothing should ever modify the content of the
# rootfs.

DATA_PATH=$SNAP_COMMON/var/lib/anbox
ROOTFS_PATH=$DATA_PATH/rootfs
RAMDISK_PATH=$DATA_PATH/ramdisk
INITRD=$SNAP/ramdisk.img
SYSTEM_IMG=$SNAP/system.img

if [ ! -e $INITRD ]; then
	echo "ERROR: boot ramdisk does not exist"
	exit 1
fi

if [ ! -e $SYSTEM_IMG ]; then
	echo "ERROR: system image does not exist"
	exit 1
fi

start() {
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

	# but certain top-level directories need to be in a writable space
	for dir in cache data; do
		mkdir -p $DATA_PATH/android-$dir
		mount -o bind $DATA_PATH/android-$dir $ROOTFS_PATH/$dir
	done

	# Make sure our setup path for the container rootfs
	# is present as lxc is statically configured for
	# this path.
	mkdir -p $SNAP_COMMON/lxc

	# We start the bridge here as long as a oneshot service unit is not
	# possible. See snapcraft.yaml for further details.
	$SNAP/bin/anbox-bridge.sh start

	exec $SNAP/usr/sbin/aa-exec -p unconfined -- $SNAP/bin/anbox-wrapper.sh container-manager
}

stop() {
	for dir in cache data; do
		umount $ROOTFS_PATH/$dir
	done
	umount $ROOTFS_PATH/system
	umount $ROOTFS_PATH

	$SNAP/bin/anbox-bridge.sh stop
}

case "$1" in
	start)
		start
		;;
	stop)
		stop
		;;
	*)
		echo "ERROR: Unknown command '$1'"
		exit 1
		;;
esac
