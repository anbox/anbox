#!/bin/bash
#
# Copyright © 2016 Canonical Ltd.
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License version 3,
# as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Taken from the Mir Project (https://launchpad.net/mir)

set -e

usage() {
  echo "usage: $(basename $0) [-a <arch>] [-c] [-h] [-d <dist>] [-u]"
  echo "  -a <arch>  Specify target architecture (armhf/arm64/powerpc/ppc64el/amd64/i386/host)"
  echo "  -c         Clean before building"
  echo "  -d <dist>  Select the distribution to build for (vivid/wily/xenial)"
  echo "  -h         This message"
  echo "  -u         Update partial chroot directory"
}

clean_build_dir() {
    rm -rf ${1}
    mkdir ${1}
}

# Default to a dist-agnostic directory name so as to not break Jenkins right now
BUILD_DIR=build-android-arm
# NUM_JOBS=$(( $(grep -c ^processor /proc/cpuinfo) + 1 ))
NUM_JOBS=5
_do_update_chroot=0

# Default to vivid as we don't seem to have any working wily devices right now 
dist=vivid
clean=0
update_build_dir=0

target_arch=armhf

while getopts "a:cd:hu" OPTNAME
do
    case $OPTNAME in
      a )
        target_arch=${OPTARG}
        update_build_dir=1
        ;;
      c )
        clean=1
        ;;
      d )
        dist=${OPTARG}
        update_build_dir=1
        ;;
      u )
        _do_update_chroot=1
        ;;
      h )
        usage
        exit 0
        ;;
      : )
        echo "Parameter -${OPTARG} needs an argument"
        usage
        exit 1;
        ;;
      * )
        echo "invalid option specified"
        usage
        exit 1
        ;;
    esac
done

shift $((${OPTIND}-1))

if [ "${target_arch}" = "host" ]; then
    target_arch=`dpkg-architecture -qDEB_HOST_ARCH`
fi

if [ ${clean} -ne 0 ]; then
    clean_build_dir ${BUILD_DIR}
fi

if [ ${update_build_dir} -eq 1 ]; then
    BUILD_DIR=build-${target_arch}-${dist}
fi

if [ "${AC_NDK_PATH}" = "" ]; then
    export AC_NDK_PATH=~/.cache/anbox-${target_arch}-chroot-${dist}
fi

if [ ! -d ${AC_NDK_PATH} ]; then 
    echo "no partial chroot dir detected. attempting to create one"
    _do_update_chroot=1
fi

if [ ! -d ${BUILD_DIR} ]; then 
    mkdir ${BUILD_DIR}
fi

echo "Building for distro: $dist"
echo "Using AC_NDK_PATH: ${AC_NDK_PATH}"

additional_repositories="-r http://ppa.launchpad.net/ci-train-ppa-service/stable-phone-overlay/ubuntu"

gcc_variant=
if [ "${dist}" = "vivid" ]; then
    gcc_variant=-4.9
fi

case ${target_arch} in
    armhf )
        target_machine=arm-linux-gnueabihf
        ;;
    amd64 )
        target_machine=x86_64-linux-gnu
        ;;
    i386 )
        target_machine=i386-linux-gnu
        ;;
    arm64 )
        target_machine=aarch64-linux-gnu
        ;;
    ppc64el )
        target_machine=powerpc64le-linux-gnu
        ;;
    powerpc )
        target_machine=powerpc-linux-gnu
        ;;
    * )
        # A good guess (assuming you have dpkg-architecture)
        target_machine=`dpkg-architecture -A${target_arch} -qDEB_HOST_MULTIARCH` || {
            echo "Unknown architecture ${target_arch}"
            usage
            exit 1
        }
        ;;
esac

echo "Target architecture: ${target_arch}"
echo "Target machine: ${target_machine}"

if [ ${_do_update_chroot} -eq 1 ] ; then
    pushd scripts > /dev/null
        ./setup-partial-chroot.sh -d ${dist} -a ${target_arch} ${additional_repositories} ${AC_NDK_PATH}
    popd > /dev/null
    # force a clean build after an update, since CMake cache maybe out of date
    clean_build_dir ${BUILD_DIR}
fi

pushd ${BUILD_DIR} > /dev/null

    export PKG_CONFIG_PATH="${AC_NDK_PATH}/usr/lib/pkgconfig:${AC_NDK_PATH}/usr/lib/${target_machine}/pkgconfig"
    export PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1
    export PKG_CONFIG_ALLOW_SYSTEM_LIBS=1
    export PKG_CONFIG_SYSROOT_DIR=$AC_NDK_PATH
    export PKG_CONFIG_EXECUTABLE=`which pkg-config`
    export AC_TARGET_MACHINE=${target_machine}
    export AC_GCC_VARIANT=${gcc_variant}
    export LDFLAGS=-Wl,-rpath-link,${AC_NDK_PATH}/usr/lib/${target_machine}/pulseaudio
    echo "Using PKG_CONFIG_PATH: $PKG_CONFIG_PATH"
    echo "Using PKG_CONFIG_EXECUTABLE: $PKG_CONFIG_EXECUTABLE"
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/LinuxCrossCompile.cmake \
      -DCMAKE_BUILD_TYPE=debug \
      ..

    make -j${NUM_JOBS} $@

popd > /dev/null
