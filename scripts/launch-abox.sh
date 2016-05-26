#!/bin/bash
set -e
set -x

basepath=/home/phablet/android-box
rootfs=$basepath/rootfs
rootfs_overrides=$basepath/overrides
ramdisk=$basepath/ramdisk.img
systemdisk=$basepath/system.img
init_cmd="/init"

if [ "$1" = "shell" ] ; then
	init_cmd=/system/bin/sh
fi

if [ -d $rootfs ] ; then
	sudo umount --recursive $rootfs || true
	rm -rf $rootfs
fi

mkdir -p $rootfs
sudo mount -t tmpfs none $rootfs
(cd $rootfs ; cat $ramdisk | gzip -d | cpio -i)

mkdir -p $rootfs/dev/pts
sudo mount -o ro,loop $systemdisk $rootfs/system

if [ -d "$rootfs_overrides" ] ; then
	for f in `ls $rootfs_overrides` ; do
		if [ "$f" = "system" ] ; then
			for f2 in `find $rootfs_overrides/system -type f` ; do
				real_path=`echo $f2 | sed -e s:$rootfs_overrides::g`
				sudo mount -o bind $f2 $rootfs/$real_path
			done
		else
			cp $rootfs_overrides/$f $rootfs
		fi
	done
fi

/home/phablet/bwrap \
	--ro-bind $rootfs / \
	--bind /home/phablet/android-box/iptables-wrapper /system/bin/iptables \
	--bind /home/phablet/android-box/iptables-wrapper /system/bin/ip6tables \
	--dev /dev \
	--proc /proc \
	--tmpfs /data \
	--unshare-user \
	--unshare-ipc \
	--unshare-pid \
	--unshare-net \
	--unshare-uts \
	--uid 0 \
	--gid 0 \
	--setenv PATH /system/bin:/system/sbin:/system/xbin \
	--chdir / \
	$init_cmd

sudo umount --recursive $rootfs
rm -rf $rootfs
