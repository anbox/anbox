#!/bin/sh
docker build -t anbox/anbox-builder .
docker run -i -t -v $PWD:/anbox anbox/anbox-builder /anbox/scripts/clean-build.sh
