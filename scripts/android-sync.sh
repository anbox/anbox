#!/bin/sh

set -ex

if [ -z "$ANDROID_PRODUCT_OUT" ]; then
	echo "ANDROID_PRODUCT_OUT is not set!"
	exit 1
fi

target=$1
if [ -z "$target" ]; then
	echo "No target path provided!"
	exit 1
fi

sudo cp -rv $ANDROID_PRODUCT_OUT/system $target
sudo rm /var/lib/anbox/rootfs/.anbox_setup_done
