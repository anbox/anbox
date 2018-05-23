#!/bin/sh

set -ex

debuild_args="$@"

if [ -z "$BUILD_ID" ]; then
	BUILD_ID=1
fi

build_and_publish_package() {
	dir="$1"
	distro="$2"
	ppa="$3"

	cd $dir

	git checkout -- debian/changelog

	# Bump the current version by a minor number. Having the distro parse
	# at the end with current date, git rev and build id will do the rest.
	new_base_version=$(dpkg-parsechangelog --show-field Version).1
	dch --newversion "$new_base_version~$(date +%Y%m%d).$BUILD_ID.$distro.$(git rev-parse --short HEAD)" \
		-D "$distro" "Nightly release"

	debuild -S -sa -I "$debuild_args"
	cd ..
	dput "$ppa" *.changes
}

top_dir="$PWD"

build_and_publish_package "$top_dir" xenial "ppa:morphis/anbox-support"
build_and_publish_package "$top_dir" bionic "ppa:morphis/anbox-support"
