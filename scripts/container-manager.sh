#!/bin/sh
set -x

# We need to put the rootfs somewhere where we can modify some
# parts of the content on first boot (namely file permissions).
# Other than that nothing should ever modify the content of the
# rootfs.

DATA_PATH=$SNAP_COMMON/var/lib/anbox
ROOTFS_PATH=$DATA_PATH/rootfs
ANDROID_IMG=$SNAP/android.img
CONTAINER_BASE_UID=100000

if [ ! -e $ANDROID_IMG ]; then
	echo "ERROR: android image does not exist"
	exit 1
fi

build_kernel_modules() {
	kversion=$1

	rm -rf $SNAP_COMMON/kernel-*

	$SNAP/bin/classic-create || true

	rm -rf $SNAP_COMMON/classic/build
	mkdir -p $SNAP_COMMON/classic/build
	cp -rav $SNAP/ashmem $SNAP_COMMON/classic/build/
	cp -rav $SNAP/binder $SNAP_COMMON/classic/build/

	cat<<EOF > $SNAP_COMMON/classic/build/run.sh
#!/bin/sh
set -ex
apt update
apt install -y --force-yes linux-headers-$kversion build-essential
cd /build/ashmem
make
cd /build/binder
make
EOF

	chmod +x $SNAP_COMMON/classic/build/run.sh
	$SNAP/bin/classic /build/run.sh

	mkdir -p $SNAP_COMMON/kernel-$kversion
	cp $SNAP_COMMON/classic/build/ashmem/ashmem_linux.ko \
		$SNAP_COMMON/kernel-$kversion/
	cp $SNAP_COMMON/classic/build/binder/binder_linux.ko \
		$SNAP_COMMON/kernel-$kversion/
}

load_kernel_modules() {
	kversion=`uname -r`
	rmmod ashmem_linux binder_linux || true
	echo "Loading kernel modules for version $kversion.."
	insmod $SNAP_COMMON/kernel-$kversion/binder_linux.ko
	insmod $SNAP_COMMON/kernel-$kversion/ashmem_linux.ko
	sleep 0.5
	chmod 666 /dev/binder
	chmod 666 /dev/ashmem
}

start() {
	# Setup the read-only rootfs
	mkdir -p $ROOTFS_PATH
	mount -o loop,ro $ANDROID_IMG $ROOTFS_PATH

	# but certain top-level directories need to be in a writable space
	for dir in cache data; do
		mkdir -p $DATA_PATH/android-$dir
		chown $CONTAINER_BASE_UID:$CONTAINER_BASE_UID $DATA_PATH/android-$dir
		mount -o bind $DATA_PATH/android-$dir $ROOTFS_PATH/$dir
	done

	# Make sure our setup path for the container rootfs
	# is present as lxc is statically configured for
	# this path.
	mkdir -p $SNAP_COMMON/lxc

	# We start the bridge here as long as a oneshot service unit is not
	# possible. See snapcraft.yaml for further details.
	$SNAP/bin/anbox-bridge.sh start

	# Building and loading the necessary kernel modules is only possible
	# on Ubuntu 16.04 (xenial)
	if [ -e /var/lib/snapd/hostfs/etc/os-release ]; then
		. /var/lib/snapd/hostfs/etc/os-release
		if [ $UBUNTU_CODENAME = xenial ]; then
			kversion=`uname -r`
			if [ ! -e $SNAP_COMMON/kernel-$kversion ]; then
				build_kernel_modules $kversion
			fi
			load_kernel_modules
		fi
	fi

	# Ensure FUSE support for user namespaces is enabled
	echo Y > /sys/module/fuse/parameters/userns_mounts

	exec $SNAP/usr/sbin/aa-exec -p unconfined -- $SNAP/bin/anbox-wrapper.sh container-manager
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
