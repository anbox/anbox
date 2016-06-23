#!/system/bin/sh
# Copyright (C) 2016 Simon Fels <morphis@gravedo.de>
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranties of
# MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
# PURPOSE.  See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program.  If not, see <http://www.gnu.org/licenses/>.

function prepare_filesystem() {
	# These dev files need to be adjusted everytime as they are
	# bind mounted into the temporary rootfs
	for f in qemu_pipe qemu_trace goldfish_pipe input/* ; do
		[ ! -e /dev/$f ] && continue
		chown system:system /dev/$f
		chmod 0666 /dev/$f
	done

	if [ ! -e /.anbox_setup_done ] ; then
		echo "Fixing up all permissions ..."

		# Fixup permissions of the android binaries in /system
		while read line
		do
			file=`echo $line | cut -d' ' -f 1`
			user=`echo $line | cut -d' ' -f 2`
			group=`echo $line | cut -d' ' -f 3`
			mode=`echo $line | cut -d' ' -f 4`
			# Avoid changing symlinks
			if [ ! -h /$file ] ; then
				chmod $mode /$file
				chown -h $user:$group /$file
			fi
		done < "/filesystem_config.txt"

		# Additional ones not listed in the config generated from the build
		for f in qemu_pipe qemu_trace goldfish_pipe ; do
			[ ! -e /dev/$f ] && continue
			chown system:system /dev/$f
			chmod 0666 /dev/$f
		done

		echo "Setup done!"
		echo $VERSION > /.anbox_setup_done
	fi
}

prepare_filesystem &
echo "Waiting for filesystem being prepared ..."
wait $!

echo "Starting real init now ..."
/init
