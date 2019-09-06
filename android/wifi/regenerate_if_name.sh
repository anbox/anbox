#!/system/bin/sh
#Script which regnerates the host wifi interface name
#from wlan0 to whatever it was before.
#As for LXC 3.0.1 a BUG prevents the regeneration from
#being renamed by LXC itself
if [ $# = "1" ]; then
  ip link set wlan0 down
  ip link set wlan0 name $1
fi
