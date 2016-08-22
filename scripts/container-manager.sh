#!/bin/bash

# Make sure our setup path for the container rootfs
# is present as lxc is statically configured for
# this path.
mkdir -p $SNAP_COMMON/lxc

exec $SNAP/bin/anbox container-manager
