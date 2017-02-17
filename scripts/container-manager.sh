#!/bin/sh
set -x

# We need to put the rootfs somewhere where we can modify some
# parts of the content on first boot (namely file permissions).
# Other than that nothing should ever modify the content of the
# rootfs.

DATA_PATH=$SNAP_COMMON/
ROOTFS_PATH=$DATA_PATH/rootfs
ANDROID_IMG=$SNAP/android.img
CONTAINER_BASE_UID=100000

if [ ! -e $ANDROID_IMG ]; then
	echo "ERROR: android image does not exist"
	exit 1
fi

load_kernel_modules() {
	kversion=`uname -r`
	rmmod ashmem_linux binder_linux || true
	echo "Loading kernel modules for version $kversion.."
	if ! `modprobe binder_linux` ; then
		echo "ERROR: Failed to load kernel binder driver"
		return
	fi
	if ! `modprobe ashmem_linux` ; then
		echo "ERROR: Failed to load kernel ashmem driver"
		return
	fi
	sleep 0.5
	chmod 666 /dev/binder
	chmod 666 /dev/ashmem
}

start() {
	# Make sure our setup path for the container rootfs
	# is present as lxc is statically configured for
	# this path.
	mkdir -p $SNAP_COMMON/lxc

	# We start the bridge here as long as a oneshot service unit is not
	# possible. See snapcraft.yaml for further details.
	$SNAP/bin/anbox-bridge.sh start

	# This will try to load the kernel modules. If this fails we will
	# continue as normal and anbox will fail later on and report a
	# visible error message to the user.
	load_kernel_modules

	# Ensure FUSE support for user namespaces is enabled
	echo Y | sudo tee /sys/module/fuse/parameters/userns_mounts || echo "WARNING: kernel doesn't support fuse in user namespaces"

	exec $SNAP/usr/sbin/aa-exec -p unconfined -- \
		$SNAP/bin/anbox-wrapper.sh container-manager \
		--data-path=$DATA_PATH \
		--android-image=$ANDROID_IMG
}

stop() {
	for dir in cache data; do
		umount $ROOTFS_PATH/$dir
	done
	umount $ROOTFS_PATH

	$SNAP/bin/anbox-bridge.sh stop

	rmmod ashmem_linux binder_linux || true
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
