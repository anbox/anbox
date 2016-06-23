#!/bin/bash

TOPDIR=`pwd`
PRODUCT_NAME=generic_arm64
OUTDIR=out/target/product/$PRODUCT_NAME

if [ -d rootfs ] ; then
	rm -rf rootfs
fi

mkdir rootfs
cp -r $OUTDIR/root/* rootfs/
cp -r $OUTDIR/system/* rootfs/system/

mkdir rootfs/cache

cp filesystem_config.txt rootfs/
cp anbox-init.sh rootfs/
chmod +x rootfs/anbox-init.sh

chmod 755 rootfs/init.*
chmod 755 rootfs/default.prop
chmod 755 rootfs/system/build.prop
chmod +x rootfs/anbox-init.sh

tar cf rootfs.tar rootfs
