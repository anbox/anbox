#!/bin/bash

TOPDIR=`echo $ANDROID_BUILD_TOP`
PRODUCT_NAME=generic_arm64
OUTDIR=out/target/product/$PRODUCT_NAME
CURDIR=`pwd`
TARGET=anbox-rootfs

if [ -d $TARGET ] ; then
	rm -rf $TARGET
fi

mkdir $TARGET
cp -r $OUTDIR/root/* $TARGET/
cp -r $OUTDIR/system/* $TARGET/system/

mkdir $TARGET/cache

find out -name filesystem_config.txt -exec cp {} $TARGET \;
if [ ! -e $TARGET/filesystem_config.txt ] ; then
	echo "ERROR: Filesystem config is not available. You have to run"
	echo "ERROR: $ make target-files-package"
	echo "ERROR: to generate it as part of the Android build."
	rm -rf $TARGET
	exit 1
fi

if [ -z "$TOPDIR" ] || [ "$CURDIR" != "$TOPDIR" ] ; then
	echo "ERROR: You have to execute this script from the ANDROID_BUILD_TOP"
	echo "ERROR: directory."
	exit 1
fi

cp anbox/scripts/anbox-init.sh $TARGET/
chmod +x $TARGET/anbox-init.sh

chmod 755 $TARGET/init.*
chmod 755 $TARGET/default.prop
chmod 755 $TARGET/system/build.prop
chmod +x $TARGET/anbox-init.sh

TARBALL_NAME=anbox-rootfs-`date +%Y%m%d%H%M`.tar
tar cf $TARBALL_NAME $TARGET
rm -rf $TARGET

echo "Created $TARBALL_NAME"
