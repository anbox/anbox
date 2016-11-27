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

varrun="/run/anbox"
varlib="/var/lib/anbox"

BRIDGE="anboxbr0"

# IPv4
IPV4_ADDR="10.0.6.1"
IPV4_NETMASK="255.255.255.0"
IPV4_NETWORK="10.0.6.1/24"
IPV4_DHCP_RANGE="10.0.6.2,10.0.6.254"
IPV4_DHCP_MAX="252"
IPV4_NAT="true"

# IPv6
IPV6_ADDR="fd9d:e4dc:4e00:9e98::1"
IPV6_MASK="64"
IPV6_NETWORK="fd9d:e4dc:4e00:9e98::1/64"
IPV6_NAT="true"
IPV6_PROXY="false"

use_iptables_lock="-w"
iptables -w -L -n > /dev/null 2>&1 || use_iptables_lock=""

HAS_IPV6=false
[ -e "/proc/sys/net/ipv6/conf/default/disable_ipv6" ] && \
    [ "$(cat /proc/sys/net/ipv6/conf/default/disable_ipv6)" = "0" ] && HAS_IPV6=true

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
        ip addr add "${CIDR_ADDR}" dev "${1}"
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

    if [ "${HAS_IPV6}" = "true" ]; then
        echo 0 > "/proc/sys/net/ipv6/conf/${BRIDGE}/autoconf" || true
        echo 0 > "/proc/sys/net/ipv6/conf/${BRIDGE}/accept_dad" || true
    fi

    # if we are run from systemd on a system with selinux enabled,
    # the mkdir will create /run/anbox as init_var_run_t which dnsmasq
    # can't write its pid into, so we restorecon it (to var_run_t)
    if [ ! -d "${varrun}" ]; then
        mkdir -p "${varrun}"
        if which restorecon >/dev/null 2>&1; then
            restorecon "${varrun}"
        fi
    fi

    if [ ! -d "${varlib}" ]; then
        mkdir -p "${varlib}"
        if which restorecon >/dev/null 2>&1; then
            restorecon "${varlib}"
        fi
    fi

    ifup "${BRIDGE}" "${IPV4_ADDR}" "${IPV4_NETMASK}"

    IPV4_ARG=""
    if [ -n "${IPV4_ADDR}" ] && [ -n "${IPV4_NETMASK}" ] && [ -n "${IPV4_NETWORK}" ]; then
        echo 1 > /proc/sys/net/ipv4/ip_forward
        if [ "${IPV4_NAT}" = "true" ]; then
            iptables "${use_iptables_lock}" -t nat -A POSTROUTING -s "${IPV4_NETWORK}" ! -d "${IPV4_NETWORK}" -j MASQUERADE -m comment --comment "managed by anbox-bridge"
        fi
        IPV4_ARG="--listen-address ${IPV4_ADDR} --dhcp-range ${IPV4_DHCP_RANGE} --dhcp-lease-max=${IPV4_DHCP_MAX}"
    fi

    IPV6_ARG=""
    if [ "${HAS_IPV6}" = "true" ] && [ -n "${IPV6_ADDR}" ] && [ -n "${IPV6_MASK}" ] && [ -n "${IPV6_NETWORK}" ]; then
        # IPv6 sysctls don't respect the "all" path...
        for interface in /proc/sys/net/ipv6/conf/*; do
            echo 2 > "${interface}/accept_ra"
        done

        for interface in /proc/sys/net/ipv6/conf/*; do
            echo 1 > "${interface}/forwarding"
        done

        ip -6 addr add dev "${BRIDGE}" "${IPV6_ADDR}/${IPV6_MASK}"
        if [ "${IPV6_NAT}" = "true" ]; then
            ip6tables "${use_iptables_lock}" -t nat -A POSTROUTING -s "${IPV6_NETWORK}" ! -d "${IPV6_NETWORK}" -j MASQUERADE -m comment --comment "managed by anbox-bridge"
        fi
        IPV6_ARG="--dhcp-range=${IPV6_ADDR},ra-stateless,ra-names --listen-address ${IPV6_ADDR}"
    fi

    iptables "${use_iptables_lock}" -I INPUT -i "${BRIDGE}" -p udp --dport 67 -j ACCEPT -m comment --comment "managed by anbox-bridge"
    iptables "${use_iptables_lock}" -I INPUT -i "${BRIDGE}" -p tcp --dport 67 -j ACCEPT -m comment --comment "managed by anbox-bridge"
    iptables "${use_iptables_lock}" -I INPUT -i "${BRIDGE}" -p udp --dport 53 -j ACCEPT -m comment --comment "managed by anbox-bridge"
    iptables "${use_iptables_lock}" -I INPUT -i "${BRIDGE}" -p tcp --dport 53 -j ACCEPT -m comment --comment "managed by anbox-bridge"
    iptables "${use_iptables_lock}" -I FORWARD -i "${BRIDGE}" -j ACCEPT -m comment --comment "managed by anbox-bridge"
    iptables "${use_iptables_lock}" -I FORWARD -o "${BRIDGE}" -j ACCEPT -m comment --comment "managed by anbox-bridge"
    iptables "${use_iptables_lock}" -t mangle -A POSTROUTING -o "${BRIDGE}" -p udp -m udp --dport 68 -j CHECKSUM --checksum-fill -m comment --comment "managed by anbox-bridge"

    DOMAIN_ARG=""
    if [ -n "${DOMAIN}" ]; then
        DOMAIN_ARG="-s ${DOMAIN} -S /${DOMAIN}/"
    fi

    CONFILE_ARG=""
    if [ -n "${CONFILE}" ]; then
        CONFILE_ARG="--conf-file=${CONFILE}"
    fi

    # https://lists.linuxcontainers.org/pipermail/lxc-devel/2014-October/010561.html
    for DNSMASQ_USER in anbox dnsmasq nobody
    do
        if getent passwd "${DNSMASQ_USER}" >/dev/null; then
            break
        fi
    done

    if [ -n "${IPV4_ADDR}" ] || [ -n "${IPV6_ADDR}" ]; then
        # shellcheck disable=SC2086
        dnsmasq ${CONFILE_ARG} ${DOMAIN_ARG} -u "${DNSMASQ_USER}" --strict-order --bind-interfaces --pid-file="${varrun}/dnsmasq.pid" --dhcp-no-override --except-interface=lo --interface="${BRIDGE}" --dhcp-leasefile="${varlib}/dnsmasq.${BRIDGE}.leases" --dhcp-authoritative ${IPV4_ARG} ${IPV6_ARG} || cleanup
    fi

    if [ "${HAS_IPV6}" = "true" ] && [ "${IPV6_PROXY}" = "true" ]; then
        PATH="${PATH}:$(dirname "${0}")" anbox-bridge-proxy --addr="[fe80::1%${BRIDGE}]:13128" &
        PID=$!
        echo "${PID}" > "${varrun}/proxy.pid"
    fi

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

        if [ "${HAS_IPV6}" = "true" ] && [ -n "${IPV6_NETWORK}" ] && [ "${IPV6_NAT}" = "true" ]; then
            ip6tables ${use_iptables_lock} -t nat -D POSTROUTING -s ${IPV6_NETWORK} ! -d ${IPV6_NETWORK} -j MASQUERADE -m comment --comment "managed by anbox-bridge"
        fi

        if [ -e "${varrun}/dnsmasq.pid" ]; then
            pid=$(cat "${varrun}/dnsmasq.pid" 2>/dev/null) && kill -9 "${pid}"
            rm -f "${varrun}/dnsmasq.pid"
        fi

        if [ -e "${varrun}/proxy.pid" ]; then
            pid=$(cat "${varrun}/proxy.pid" 2>/dev/null) && kill -9 "${pid}"
            rm -f "${varrun}/proxy.pid"
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
