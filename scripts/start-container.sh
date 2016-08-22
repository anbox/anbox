#!/bin/bash

exec $SNAP/bin/lxc-start -P $SNAP_COMMON/var/lib/anbox/containers -n default -F
