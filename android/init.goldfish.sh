#!/system/bin/sh

#.host_network_info is created from the snap environment
if_name=$(tail -n 1 /data/.host_network_info)
driver=$(head -n 1 /data/.host_network_info)
#Setting Android variable for use in init.goldfish.rc
setprop wifi.wpa.driver $driver

# Setup networking when boot starts
if [ "$driver" = "wired" ]; then
    link_type="eth"
else
    link_type="wifi"
fi

#Check if Link type has changed and only then rewrite wpa conf
#otherwise user would loose saved wifi networks on every boot
if ! [ -e /data/misc/wifi/.old_link_type ] || ! [ $(head -n 1 /data/misc/wifi/.old_link_type) = ${link_type} ]; then
    cp /system/etc/wifi/wpa_supplicant_${link_type}.conf /data/misc/wifi/wpa_supplicant.conf
fi
echo ${link_type} > /data/misc/wifi/.old_link_type

chmod 0660 /data/misc/wifi/wpa_supplicant.conf
chown wifi:wifi /data/misc/wifi/wpa_supplicant.conf

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
