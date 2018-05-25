#!/bin/bash

COMPILERS_CXX98=`cat<<EOF
gcc-4.4
gcc-4.6
gcc-4.7
gcc-4.8
clang
EOF`

COMPILERS_CXX11=`cat<<EOF
gcc-4.7
gcc-4.8
clang
EOF`


function mkbuild() {
	local compiler=$1
	local lang=$2
	local buildtype=$3
	local builddir="$4"
	export CC=$compiler
	export CXX=`echo $compiler | sed -e 's/clang/clang++/' -e 's/gcc/g++/'`
	export CXXFLAGS="-std=$lang"
	echo "Creating $builddir"
	mkdir $builddir 2>/dev/null
	(
		cd "$builddir"
		cmake -DCMAKE_BUILD_TYPE=$buildtype ..
	)
}

function build() {
	local builddir=$1
	shift
	make -C "$builddir" $@
}

function dotest() {
	local builddir=$1
	shift
	make -C "$builddir" test $@
	return 0
}

function do_action() {
	local lang=$1
	local action=$2
	shift 2

	for compiler in $COMPILERS; do
		local builddir="build_${lang}_${compiler}"

		if [[ $action == "cmake" ]]; then
			buildtype=$1
			mkbuild $compiler $lang "$buildtype" "$builddir"
			[[ $? != 0 ]] && exit
		elif [[ $action == "make" ]]; then
			build "$builddir" $@
			[[ $? != 0 ]] && exit
		elif [[ $action == "test" ]]; then
			dotest "$builddir" $@
			[[ $? != 0 ]] && exit
		elif [[ $action == "clean" ]]; then
			rm -r "$builddir"
		else
			echo "usage: $0 cmake [debug|release|relwithdbg]|make|test|clean"
			exit 255
		fi
	done
}

COMPILERS=$COMPILERS_CXX98
do_action c++98 $@
COMPILERS=$COMPILERS_CXX11
do_action c++11 $@
