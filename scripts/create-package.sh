#!/bin/bash

TOPDIR=`echo $ANDROID_BUILD_TOP`
PRODUCT_NAME=generic_arm64
OUTDIR=out/target/product/$PRODUCT_NAME
CURDIR=`pwd`

if [ -d rootfs ] ; then
	rm -rf rootfs
fi

mkdir rootfs
cp -r $OUTDIR/root/* rootfs/
cp -r $OUTDIR/system/* rootfs/system/

mkdir rootfs/cache

find out -name filesystem_config.txt -exec cp {} rootfs \;
if [ ! -e rootfs/filesystem_config.txt ] ; then
	echo "ERROR: Filesystem config is not available. You have to run"
	echo "ERROR: $ make target-files-package"
	echo "ERROR: to generate it as part of the Android build."
	rm -rf rootfs
	exit 1
fi

if [ -z "$TOPDIR" ] || [ "$CURDIR" != "$TOPDIR" ] ; then
	echo "ERROR: You have to execute this script from the ANDROID_BUILD_TOP"
	echo "ERROR: directory."
	exit 1
fi

cp anbox-init.sh rootfs/
chmod +x rootfs/anbox-init.sh

chmod 755 rootfs/init.*
chmod 755 rootfs/default.prop
chmod 755 rootfs/system/build.prop
chmod +x rootfs/anbox-init.sh

tar cf rootfs.tar rootfs
rm -rf rootfs
