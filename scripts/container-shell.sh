#!/bin/bash

$SNAP/bin/lxc-attach \
	--lxcpath=$SNAP_COMMON/var/lib/anbox/containers \
	--name default \
	--clear-env \
	--set-var PATH=/system/bin:/system/sbin:/system/xbin \
	--set-var ANDROID_DATA=/data \
	-- \
	/system/bin/sh
