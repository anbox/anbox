#!/bin/sh
docker build -t anbox/anbox-builder .
docker run --rm -v $PWD:/anbox anbox/anbox-builder /anbox/scripts/clean-build.sh
