#!/bin/bash
set -x

# We need to put the rootfs somewhere where we can modify some
# parts of the content on first boot (namely file permissions).
# Other than that nothing should ever modify the content of the
# rootfs.

DATA_PATH=$SNAP_COMMON/
ANDROID_IMG=$SNAP/android.img

if [ "$(id -u)" != 0 ]; then
	echo "ERROR: You need to run the container manager as root"
	exit 1
fi

if [ ! -e "$ANDROID_IMG" ]; then
	echo "ERROR: android image does not exist"
	exit 1
fi

if [ "$SNAP_ARCH" = "amd64" ]; then
	ARCH="x86_64-linux-gnu"
elif [ "$SNAP_ARCH" = "armhf" ]; then
	ARCH="arm-linux-gnueabihf"
else
	ARCH="$SNAP_ARCH-linux-gnu"
fi

# Re-exec outside of apparmor confinement
if [ -d /sys/kernel/security/apparmor ] && [ "$(cat /proc/self/attr/current)" != "unconfined" ]; then
	exec /usr/sbin/aa-exec -p unconfined -- "$0" "$@"
fi

start() {
	# Make sure our setup path for the container rootfs
	# is present as lxc is statically configured for
	# this path.
	mkdir -p "$SNAP_COMMON/lxc"

	# We start the bridge here as long as a oneshot service unit is not
	# possible. See snapcraft.yaml for further details.
	"$SNAP"/bin/anbox-bridge.sh start

	# Ensure FUSE support for user namespaces is enabled
	echo Y | tee /sys/module/fuse/parameters/userns_mounts || echo "WARNING: kernel doesn't support fuse in user namespaces"

	# liblxc.so.1 is in $SNAP/lib
	export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$SNAP/liblxc"

	# For unknown reason we got bug reports that the container manager failed to start
	# because it cannot find libboost_log.so.1.58.0 To mitigate this we're adding the
	# lib directory as explicit search target here.
	export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$SNAP/usr/lib/$ARCH"

	enable_debug="$(snapctl get debug.enable)"
	if [ "$enable_debug" = true ]; then
		export ANBOX_LOG_LEVEL=debug
		export LD_DEBUG=libs
	fi

	EXTRA_ARGS=
	enable_rootfs_overlay="$(snapctl get rootfs-overlay.enable)"
	if [ "$enable_rootfs_overlay" = true ]; then
		EXTRA_ARGS="$EXTRA_ARGS --use-rootfs-overlay"
	fi

	enable_privileged_container="$(snapctl get container.privileged)"
	if [ "$enable_privileged_container" = true ]; then
		EXTRA_ARGS="$EXTRA_ARGS --privileged"
	fi

	container_network_address=$(snapctl get container.network.address)
	if [ -n "$container_network_address" ]; then
		EXTRA_ARGS="$EXTRA_ARGS --container-network-address=$container_network_address"
	fi

	container_network_gateway=$(snapctl get container.network.gateway)
	if [ -n "$container_network_gateway" ]; then
		EXTRA_ARGS="$EXTRA_ARGS --container-network-gateway=$container_network_gateway"
	fi

	container_network_dns=$(snapctl get container.network.dns)
	if [ -n "$container_network_dns" ]; then
		EXTRA_ARGS="$EXTRA_ARGS --container-network-dns-servers=$container_network_dns"
	fi

	exec "$SNAP"/bin/anbox-wrapper.sh container-manager \
		--data-path="$DATA_PATH" \
		--android-image="$ANDROID_IMG" \
		--daemon \
		$EXTRA_ARGS
}

stop() {
	"$SNAP"/bin/anbox-bridge.sh stop
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
