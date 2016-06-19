#!/bin/sh

set -e
set -x

if [ ! -e android-rootfs.tar ] ; then
	echo "ERROR: Missing Android rootfs package!"
	exit 1
fi

./cross-compile-chroot.sh -a armhf -d vivid

if [ -e click ] ; then
	rm -rf click
fi

mkdir click
cp data/manifest.json click/
cp data/apparmor.json click/
cp data/anbox.desktop click/

(cd click ; tar xf ../android-rootfs.tar ; mv rootfs android-rootfs)

cp build-armhf-vivid/src/anbox click/
cp build-armhf-vivid/src/anbox-container click/

(cd click ; click build .)
