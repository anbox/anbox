#!/bin/bash

DATA_PATH=/var/lib/anbox
ROOTFS_PATH=$DATA_PATH/rootfs
ANDROID_IMG=$1
CONTAINER_BASE_UID=100000
DIR="$(dirname $0)"

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

if [ -z "$1" ]; then
    echo "Usage:"
    echo "./mount-android.sh [Android image]"
    exit 1
fi

if [ ! -f $ANDROID_IMG ]; then
    echo "Android image not found"
    exit 1
fi

# Setup the read-only rootfs
mkdir -p $ROOTFS_PATH
mount -o loop,ro $ANDROID_IMG $ROOTFS_PATH

# but certain top-level directories need to be in a writable space
for dir in cache data; do
  mkdir -p $DATA_PATH/android-$dir
  chown $CONTAINER_BASE_UID:$CONTAINER_BASE_UID $DATA_PATH/android-$dir
  mount -o bind $DATA_PATH/android-$dir $ROOTFS_PATH/$dir
done

$DIR/anbox-bridge.sh start
$DIR/load-kmods.sh
