#!/bin/sh
docker pull ubuntu:16.04
docker run -i -t -v $PWD:/anbox ubuntu:16.04 /anbox/scripts/clean-build.sh
