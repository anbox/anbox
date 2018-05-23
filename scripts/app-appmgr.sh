#!/bin/sh

PACKAGE=org.anbox.appmgr
COMPONENT=org.anbox.appmgr.AppViewActivity

exec $SNAP/bin/anbox-wrapper.sh launch \
	--package="$PACKAGE" \
	--component="$COMPONENT"
