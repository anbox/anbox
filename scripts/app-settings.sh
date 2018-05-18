#!/bin/sh

PACKAGE=com.android.settings

exec $SNAP/bin/anbox-wrapper.sh launch \
	--package="$PACKAGE"
