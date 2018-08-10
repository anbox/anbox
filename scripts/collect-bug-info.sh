#!/bin/sh -e

TMPDIR=$(mktemp -d)

if [ $(id -u) -ne 0 ] ; then
	echo "WARNING: You're not running as root. This will prevent the script from"
	echo "         adding a few important file for bug diagnostics to the report."
	echo "         If you want to have a look at the script before giving it root"
	echo "         access, please have a look at $0."
	exit 0
fi

echo "This script will collect a few interesting things which developers will"
echo "need to have a better insight into the Anbox system when something goes"
echo "wrong."
echo
echo "PLEASE NOTE: The collected log files may collect information about your"
echo "system. Please have a look before you're sending them to anyone!"
echo
echo "Collecting anbox log files ... "

set -x
# Collect several things which are of interest for bug reports
cp /var/snap/anbox/common/data/system.log $TMPDIR || true
cp /var/snap/anbox/common/containers/lxc-monitord.log $TMPDIR || true
cp /var/snap/anbox/common/logs/container.log $TMPDIR || true
cp /var/snap/anbox/common/logs/console.log* $TMPDIR || true
$SNAP/command-anbox.wrapper system-info > $TMPDIR/system-info.log 2>&1 || true

if [ -e /etc/systemd/system/snap.anbox.container-manager.service ]; then
	sudo journalctl --no-pager -u snap.anbox.container-manager.service > $TMPDIR/container-manager.log 2>&1 || true
fi
set +x

echo
echo "DONE!"

echo
echo "Do you want to include the kernel log of your system? [y/N]"
read -r action
if [ "$action" = "y" ]; then
	set -x
	dmesg > $TMPDIR/kernel.log 2>&1
	set +x
fi

echo "Generating archive with all log files in $PWD ..."
CURDIR=$PWD
(cd $TMPDIR; zip -r $CURDIR/anbox-system-diagnostics-$(date --rfc-3339=date --utc).zip *)
rm -rf $TMPDIR
echo "DONE!"

echo
echo "Now please take the ZIP archive generated in your current directory"
echo "and attach it to your bug report. Please don't hesitate to have a"
echo "look into the archive before you do so to verify you don't leak any"
echo "information you don't want!"
