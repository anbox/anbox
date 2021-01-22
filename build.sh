#!/bin/bash

help() {
        echo "build.sh [options] -- [cmake args]"
        echo ""
        echo "Options:"
        echo "--clean      : remove contents of directory ./build"
        echo "               fully rebuild the project"

        echo "--hard-reset : reset current directory to last commit, implies --clean"
        echo "               In cases where anbox comes directly from a checked out Android"
        echo "               build environment we miss some symlinks which are present on the"
        echo "               host and don't have a valid git repository in that case."

        echo "--docker     : use a docker container to build Anbox"
        
        echo "-j, --jobs=X : number of parallel make jobs, default is 8"
        echo "               equal to 'make --jobs=X'"
}


# Option strings
SHORT=hj:
LONG=help,clean,docker,hard-reset,jobs:

# read the options
OPTS=$(getopt --options $SHORT --long $LONG --name "$0" -- "$@")
if [ $? != 0 ] ; then 
    echo "Failed to parse options...exiting." >&2
    exit 1
fi

eval set -- "$OPTS"


DO_CLEAN=false
DO_RESET=false
USE_DOCKER=false

MAKE_JOBS=8
CMAKE_ARGS=""

BUILD_DIR=${PWD}/build
SOURCE_DIR=${PWD}

# extract options and their arguments into variables.
while true ; do
  case "$1" in
    # --------------------------------------------------
    -h | --help )
        help; exit 0;;
    # --------------------------------------------------
    --clean )
        DO_CLEAN=true; shift;;
    # --------------------------------------------------
    --docker )
        USE_DOCKER=true; shift;;
    # --------------------------------------------------
    -j | --jobs )
        if [ $2 -gt 0 ]; then MAKE_JOBS=$2; fi
        shift 2;;
    # --------------------------------------------------
    --hard-reset )
        if [ -d .git ] ; then DO_RESET=true; fi
        shift;;
    # --------------------------------------------------
    -- )
        shift; CMAKE_ARGS=$@;
        break;;
    # --------------------------------------------------
    *)
        echo "Internal error!"; 
        exit 1;;
  esac
done

if [ $DO_RESET == true ]; then
    DO_CLEAN=true
    echo "performing hard reset"

    git clean -fdx .
    git reset --hard
    git submodule init
    git submodule update
fi


if [ $DO_CLEAN == true ]; then
    echo "performing clean"

    # this will leave files and folders starting with '.', like '.cmake'
    if [ -d build ]; then rm -fr $BUILD_DIR/*; fi
fi


# create build directory if necessary
if [ ! -d $BUILD_DIR ]; then mkdir -p $BUILD_DIR; fi


if [ $USE_DOCKER == true ]; then
    # enable buildkit for mutli-stage image builds
    DOCKER_BUILDKIT=1 docker build --tag anbox/anbox-builder $SOURCE_DIR
    
    docker run --tty \
        --user=$(id -u):$(id -g) \
        --volume=$SOURCE_DIR:/anbox \
        --volume=$BUILD_DIR:/anbox/build \
        anbox/anbox-builder \
        /bin/sh -c "cd /anbox/build && cmake .. $CMAKE_ARGS && make --jobs=$MAKE_JOBS && make tests"
else
    cmake -S$SOURCE_DIR -B$BUILD_DIR $CMAKE_ARGS 
    make --directory=$BUILD_DIR --jobs=$MAKE_JOBS
fi
