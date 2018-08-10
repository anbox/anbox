#!/bin/sh

if [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: You need to run this script as root!"
    exit 1
fi

CONTAINER_PATH="$SNAP_COMMON/containers"
CONTAINER_NAME=default

if [ -z "$SNAP" ] ; then
    CONTAINER_PATH=/var/lib/anbox/containers
fi

state=$(lxc-info -P "$CONTAINER_PATH" -n "$CONTAINER_NAME" -s | cut -d':' -f 2 | tr -d '[:space:]')
if [ "$state" != "RUNNING" ] ; then
    echo "ERROR: Cannot attach to container as it is not running"
    exit 1
fi

echo "NOTE: This shell will give you full root access inside the Android"
echo "      container. Be careful with what you're doing!"
echo

exec lxc-attach \
    -q \
    --clear-env \
    -P "$CONTAINER_PATH" \
    -n "$CONTAINER_NAME" \
    -v PATH=/sbin:/system/bin:/system/sbin:/system/xbin \
    -v ANDROID_ASSETS=/assets \
    -v ANDROID_DATA=/data \
    -v ANDROID_ROOT=/system \
    -v ANDROID_STORAGE=/storage \
    -v ASEC_MOUNTPOINT=/mnt/asec \
    -v EXTERNAL_STORAGE=/sdcard -- /system/bin/sh
