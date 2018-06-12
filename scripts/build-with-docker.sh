#!/bin/sh
docker pull ubuntu:18.04
docker run -i -t -v $PWD:/anbox ubuntu:18.04 /anbox/scripts/clean-build.sh
