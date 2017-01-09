#!/system/bin/sh

# Setup networking when boot starts
ifconfig eth0 10.0.2.15 netmask 255.255.255.0 up
route add default gw 10.0.2.2 dev eth0

# ro.kernel.android.qemud is normally set when we
# want the RIL (radio interface layer) to talk to
# the emulated modem through qemud.
#
# However, this will be undefined in two cases:
#
# - When we want the RIL to talk directly to a guest
#   serial device that is connected to a host serial
#   device by the emulator.
#
# - We don't want to use the RIL but the VM-based
#   modem emulation that runs inside the guest system
#   instead.
#
# The following detects the latter case and sets up the
# system for it.
#
qemud=`getprop ro.kernel.android.qemud`
case "$qemud" in
    "")
    radio_ril=`getprop ro.kernel.android.ril`
    case "$radio_ril" in
        "")
        # no need for the radio interface daemon
        # telephony is entirely emulated in Java
        setprop ro.radio.noril yes
        stop ril-daemon
        ;;
    esac
    ;;
esac

# Setup additionnal DNS servers if needed
num_dns=`getprop ro.kernel.ndns`
case "$num_dns" in
    2) setprop net.eth0.dns2 10.0.2.4
       ;;
    3) setprop net.eth0.dns2 10.0.2.4
       setprop net.eth0.dns3 10.0.2.5
       ;;
    4) setprop net.eth0.dns2 10.0.2.4
       setprop net.eth0.dns3 10.0.2.5
       setprop net.eth0.dns4 10.0.2.6
       ;;
esac

# disable boot animation for a faster boot sequence when needed
boot_anim=`getprop ro.kernel.android.bootanim`
case "$boot_anim" in
    0)  setprop debug.sf.nobootanimation 1
    ;;
esac

# set up the second interface (for inter-emulator connections)
# if required
my_ip=`getprop net.shared_net_ip`
case "$my_ip" in
    "")
    ;;
    *) ifconfig eth1 "$my_ip" netmask 255.255.255.0 up
    ;;
esac
