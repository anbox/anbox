#!/usr/bin/env bash
# Use bubblewrap to run /bin/sh in the host's rootfs.
set -euo pipefail
(exec bwrap --ro-bind /usr /usr \
      --dir /tmp \
      --proc /proc \
      --dev /dev \
      --ro-bind /etc/resolv.conf /etc/resolv.conf \
      --symlink usr/lib /lib \
      --symlink usr/lib64 /lib64 \
      --symlink usr/bin /bin \
      --symlink usr/sbin /sbin \
      --chdir / \
      --unshare-pid \
      --dir /run/user/$(id -u) \
      --setenv XDG_RUNTIME_DIR "/run/user/`id -u`" \
      --file 11 /etc/passwd \
      --file 12 /etc/group \
      /bin/sh) \
    11< <(getent passwd $UID 65534) \
    12< <(getent group $(id -g) 65534)
