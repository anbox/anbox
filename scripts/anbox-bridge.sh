#!/bin/sh
# Copyright (C) 2016 The Linux Containers Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Taken from https://github.com/lxc/lxd-pkg-ubuntu/blob/dpm-xenial/lxd-bridge/lxd-bridge
# but modified for the use within anbox.

varrun="/run/anbox"

if [ -n "$SNAP_DATA" ]; then
    varrun="$SNAP_DATA"/network
fi

BRIDGE="anbox0"

# IPv4
IPV4_ADDR="192.168.250.1"
IPV4_NETMASK="255.255.255.0"
IPV4_NETWORK="192.168.250.1/24"
IPV4_BROADCAST="0.0.0.0"
IPV4_NAT="true"

if [ -n "$SNAP" ]; then
    snap_ipv4_address=$(snapctl get bridge.address)
    snap_ipv4_netmask=$(snapctl get bridge.netmask)
    snap_ipv4_network=$(snapctl get bridge.network)
    snap_ipv4_broadcast=$(snapctl get bridge.broadcast)
    snap_enable_nat=$(snapctl get bridge.nat.enable)
    if [ -n "$snap_ipv4_address" ]; then
        IPV4_ADDR="$snap_ipv4_address"
    fi
    if [ -n "$snap_ipv4_netmask" ]; then
        IPV4_NETMASK="$snap_ipv4_netmask"
    fi
    if [ -n "$snap_ipv4_network" ]; then
        IPV4_NETWORK="$snap_ipv4_network"
    fi
    if [ -n "$snap_ipv4_broadcast" ]; then
        IPV4_BROADCAST="$snap_ipv4_broadcast"
    fi
    if [ "$snap_enable_nat" = false ]; then
        IPV4_NAT="false"
    fi
fi

use_iptables_lock="-w"
iptables -w -L -n > /dev/null 2>&1 || use_iptables_lock=""

_netmask2cidr () {
    # Assumes there's no "255." after a non-255 byte in the mask
    local x=${1##*255.}
    set -- "0^^^128^192^224^240^248^252^254^" "$(( (${#1} - ${#x})*2 ))" "${x%%.*}"
    x=${1%%${3}*}
    echo $(( ${2} + (${#x}/4) ))
}

ifdown() {
    ip addr flush dev "${1}"
    ip link set dev "${1}" down
}

ifup() {
    [ "${HAS_IPV6}" = "true" ] && [ "${IPV6_PROXY}" = "true" ] && ip addr add fe80::1/64 dev "${1}"
    if [ -n "${IPV4_NETMASK}" ] && [ -n "${IPV4_ADDR}" ]; then
        MASK=$(_netmask2cidr ${IPV4_NETMASK})
        CIDR_ADDR="${IPV4_ADDR}/${MASK}"
	ip addr add "${CIDR_ADDR}" broadcast "${IPV4_BROADCAST}" dev "${1}"
    fi
    ip link set dev "${1}" up
}

start() {
    [ ! -f "${varrun}/network_up" ] || { echo "anbox-bridge is already running"; exit 1; }

    if [ -d /sys/class/net/${BRIDGE} ]; then
        stop force 2>/dev/null || true
    fi

    FAILED=1

    cleanup() {
        set +e
        if [ "${FAILED}" = "1" ]; then
            echo "Failed to setup anbox-bridge." >&2
            stop force
        fi
    }

    trap cleanup EXIT HUP INT TERM
    set -e

    # set up the anbox network
    [ ! -d "/sys/class/net/${BRIDGE}" ] && ip link add dev "${BRIDGE}" type bridge

    # if we are run from systemd on a system with selinux enabled,
    # the mkdir will create /run/anbox as init_var_run_t which dnsmasq
    # can't write its pid into, so we restorecon it (to var_run_t)
    if [ ! -d "${varrun}" ]; then
        mkdir -p "${varrun}"
        if which restorecon >/dev/null 2>&1; then
            restorecon "${varrun}"
        fi
    fi

    ifup "${BRIDGE}" "${IPV4_ADDR}" "${IPV4_NETMASK}"

    IPV4_ARG=""
    if [ -n "${IPV4_ADDR}" ] && [ -n "${IPV4_NETMASK}" ] && [ -n "${IPV4_NETWORK}" ]; then
        echo 1 > /proc/sys/net/ipv4/ip_forward
        if [ "${IPV4_NAT}" = "true" ]; then
            iptables "${use_iptables_lock}" -t nat -A POSTROUTING -s "${IPV4_NETWORK}" ! -d "${IPV4_NETWORK}" -j MASQUERADE -m comment --comment "managed by anbox-bridge"
        fi
    fi

    iptables "${use_iptables_lock}" -I INPUT -i "${BRIDGE}" -p udp --dport 67 -j ACCEPT -m comment --comment "managed by anbox-bridge"
    iptables "${use_iptables_lock}" -I INPUT -i "${BRIDGE}" -p tcp --dport 67 -j ACCEPT -m comment --comment "managed by anbox-bridge"
    iptables "${use_iptables_lock}" -I INPUT -i "${BRIDGE}" -p udp --dport 53 -j ACCEPT -m comment --comment "managed by anbox-bridge"
    iptables "${use_iptables_lock}" -I INPUT -i "${BRIDGE}" -p tcp --dport 53 -j ACCEPT -m comment --comment "managed by anbox-bridge"
    iptables "${use_iptables_lock}" -I FORWARD -i "${BRIDGE}" -j ACCEPT -m comment --comment "managed by anbox-bridge"
    iptables "${use_iptables_lock}" -I FORWARD -o "${BRIDGE}" -j ACCEPT -m comment --comment "managed by anbox-bridge"
    iptables "${use_iptables_lock}" -t mangle -A POSTROUTING -o "${BRIDGE}" -p udp -m udp --dport 68 -j CHECKSUM --checksum-fill -m comment --comment "managed by anbox-bridge"

    touch "${varrun}/network_up"
    FAILED=0
}

stop() {
    [ -f "${varrun}/network_up" ] || [ "${1}" = "force" ] || { echo "anbox-bridge isn't running"; exit 1; }

    if [ -d /sys/class/net/${BRIDGE} ]; then
        ifdown ${BRIDGE}
        iptables ${use_iptables_lock} -D INPUT -i ${BRIDGE} -p udp --dport 67 -j ACCEPT -m comment --comment "managed by anbox-bridge"
        iptables ${use_iptables_lock} -D INPUT -i ${BRIDGE} -p tcp --dport 67 -j ACCEPT -m comment --comment "managed by anbox-bridge"
        iptables ${use_iptables_lock} -D INPUT -i ${BRIDGE} -p udp --dport 53 -j ACCEPT -m comment --comment "managed by anbox-bridge"
        iptables ${use_iptables_lock} -D INPUT -i ${BRIDGE} -p tcp --dport 53 -j ACCEPT -m comment --comment "managed by anbox-bridge"
        iptables ${use_iptables_lock} -D FORWARD -i ${BRIDGE} -j ACCEPT -m comment --comment "managed by anbox-bridge"
        iptables ${use_iptables_lock} -D FORWARD -o ${BRIDGE} -j ACCEPT -m comment --comment "managed by anbox-bridge"
        iptables ${use_iptables_lock} -t mangle -D POSTROUTING -o ${BRIDGE} -p udp -m udp --dport 68 -j CHECKSUM --checksum-fill -m comment --comment "managed by anbox-bridge"

        if [ -n "${IPV4_NETWORK}" ] && [ "${IPV4_NAT}" = "true" ]; then
            iptables ${use_iptables_lock} -t nat -D POSTROUTING -s ${IPV4_NETWORK} ! -d ${IPV4_NETWORK} -j MASQUERADE -m comment --comment "managed by anbox-bridge"
        fi

        # if ${BRIDGE} has attached interfaces, don't destroy the bridge
        ls /sys/class/net/${BRIDGE}/brif/* > /dev/null 2>&1 || ip link delete "${BRIDGE}"
    fi

    rm -f "${varrun}/network_up"
}

# See how we were called.
case "${1}" in
    start)
        start
    ;;

    stop)
        stop
    ;;

    restart|reload|force-reload)
        ${0} stop
        ${0} start
    ;;

    *)
        echo "Usage: ${0} {start|stop|restart|reload|force-reload}"
        exit 2
esac

exit $?
