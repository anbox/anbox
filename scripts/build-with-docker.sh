#!/bin/sh
docker build -t anbox/anbox-builder .
docker run --rm --user=$(id -u):$(id -g) --volume=$PWD:/anbox anbox/anbox-builder /anbox/scripts/build.sh $@
