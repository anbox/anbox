# ---- BASE IMAGE
FROM ubuntu:20.04 as base

RUN apt-get update && \
  DEBIAN_FRONTEND="noninteractive" apt-get install --no-install-recommends --yes \
    build-essential \
    cmake


# ---- COMPILE SDBUS-C++
FROM base as sdbus-cpp

RUN DEBIAN_FRONTEND="noninteractive" apt-get install --no-install-recommends --yes \
  build-essential \
  libsystemd-dev \
  pkg-config \
  libexpat1-dev

# copy and build library sdbus-cpp
RUN mkdir -p /tmp/
COPY external/sdbus-cpp /tmp/sdbus-cpp
RUN   cmake -S/tmp/sdbus-cpp -B/tmp/sdbus-cpp/build \
        -DBUILD_CODE_GEN=ON \
        -DBUILD_SHARED_LIBS=OFF \
    && make --directory=/tmp/sdbus-cpp/build --jobs=8
    


# ---- COMPILE ANBOX
FROM base as anbox

# hadolint ignore=DL3008
RUN DEBIAN_FRONTEND="noninteractive" apt-get install --no-install-recommends --yes \
  ca-certificates \
  cmake-data \
  cmake-extras \
  debhelper \
  dbus \
  git \
  google-mock \
  libboost-dev \
  libboost-filesystem-dev \
  libboost-log-dev \
  libboost-iostreams-dev \
  libboost-program-options-dev \
  libboost-system-dev \
  libboost-test-dev \
  libboost-thread-dev \
  libcap-dev \
  libegl1-mesa-dev \
  libexpat1-dev \
  libgles2-mesa-dev \
  libglm-dev \
  libgtest-dev \
  liblxc1 \
  libproperties-cpp-dev \
  libprotobuf-dev \
  libsdl2-dev \
  libsdl2-image-dev \
  libsystemd-dev \
  lxc-dev \
  python3 \
  protobuf-compiler && \
  apt-get clean && \
  rm -rf /var/lib/apt/lists/*

# install sdbus-cpp and add to path
COPY --from=sdbus-cpp /tmp/sdbus-cpp /tmp/sdbus-cpp
RUN make --directory=/tmp/sdbus-cpp/build install
ENV PATH="/usr/local/bin/sdbus-c++-xml2cpp:${PATH}"

WORKDIR /anbox
