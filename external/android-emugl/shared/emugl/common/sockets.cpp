// Copyright (C) 2014 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "emugl/common/sockets.h"

#include <errno.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <stdio.h>
#endif

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

namespace emugl {

namespace {

static void socketSetDontLinger(int s) {
#ifdef _WIN32
  // TODO: Verify default behavior on WINDOWS
#else
    // Ungraceful shutdown, no reason to linger at all
    struct linger so_linger;
    so_linger.l_onoff  = 1;
    so_linger.l_linger = 0;
    if(setsockopt(s, SOL_SOCKET, SO_LINGER, &so_linger, sizeof so_linger) < 0)
      perror("Setting socket option SO_LINGER={on, 0} failed");
#endif
}

static void socketSetReuseAddress(int s) {
#ifdef _WIN32
    // The default behaviour on Windows is equivalent to SO_REUSEADDR
    // so we don't need to set this option. Moreover, one should never
    // set this option with WinSock because it's badly implemented and
    // generates a huge security issue. See:
    // http://msdn.microsoft.com/en-us/library/windows/desktop/ms740621(v=vs.85).aspx
#else
    int val = 1;
    if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0)
      perror("Setting socket option SO_REUSEADDR failed");
#endif
}

// Helper union to store a socket address.
struct SockAddr {
    socklen_t len;
    union {
        sockaddr generic;
        sockaddr_in inet;
#ifndef _WIN32
        sockaddr_un local;
#endif
    };

    int getFamily() const { return generic.sa_family; }

    void initEmpty() {
        ::memset(this, 0, sizeof(*this));
        this->len = static_cast<socklen_t>(sizeof(*this));
    }

    int initFromInet(uint32_t ip_address, int port) {
        if (port < 0 || port >= 65536)
            return -EINVAL;

        ::memset(this, 0, sizeof(*this));
        this->inet.sin_family = AF_INET;
        this->inet.sin_port = htons(port);
        this->inet.sin_addr.s_addr = htonl(ip_address);
        this->len = sizeof(this->inet);
        return 0;
    }

    int initFromLocalhost(int port) {
        return initFromInet(0x7f000001, port);
    }

#ifndef _WIN32
    // Initialize the SockAddr from a Unix path. Returns 0 on success,
    // or -errno code on failure.
    int initFromUnixPath(const char* path) {
        if (!path || !path[0])
            return -EINVAL;

        size_t pathLen = ::strlen(path);
        if (pathLen >= sizeof(local.sun_path))
            return -E2BIG;

        ::memset(this, 0, sizeof(*this));
        this->local.sun_family = AF_LOCAL;
        ::memcpy(this->local.sun_path, path, pathLen + 1U);
        this->len = pathLen + offsetof(sockaddr_un, sun_path);
        return 0;
    }
#endif
};

int socketBindInternal(const SockAddr* addr, int socketType) {
    int s = ::socket(addr->getFamily(), socketType, 0);
    if (s < 0) {
        perror("Could not create socket to bind");
        return -errno;
    }

    socketSetDontLinger(s);
    socketSetReuseAddress(s);

    // Bind to the socket.
    if (::bind(s, &addr->generic, addr->len) < 0 ||
        ::listen(s, 5) < 0) {
        int ret = -errno;
        perror("Could not bind or listen to socket");
        ::close(s);
        return ret;
    }

    return s;
}

int socketConnectInternal(const SockAddr* addr, int socketType) {
    int s = ::socket(addr->getFamily(), socketType, 0);
    if (s < 0) {
        perror("Could not create socket to connect");
        return -errno;
    }

    socketSetDontLinger(s);
    socketSetReuseAddress(s);

    int ret;
    do {
        ret = ::connect(s, &addr->generic, addr->len);
    } while (ret < 0 && errno == EINTR);

    if (ret < 0) {
        ret = -errno;
        ::close(s);
        return ret;
    }

    return s;
}

}  // namespace

void socketTcpDisableNagle(int s) {
    // disable Nagle algorithm to improve bandwidth of small
    // packets which are quite common in our implementation.
#ifdef _WIN32
    DWORD  flag;
#else
    int    flag;
#endif
    flag = 1;
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY,
               (const char*)&flag, sizeof(flag));
}

int socketGetPort(int s) {
    SockAddr addr;
    addr.initEmpty();
    if (getsockname(s, &addr.generic, &addr.len) < 0) {
        return -errno;
    }
    switch (addr.generic.sa_family) {
        case AF_INET:
            return ntohs(addr.inet.sin_port);
        default:
            ;
    }
    return -EINVAL;
}

#ifndef _WIN32
int socketLocalServer(const char* path, int socketType) {
    SockAddr addr;
    int ret = addr.initFromUnixPath(path);
    if (ret < 0) {
        return ret;
    }
    return socketBindInternal(&addr, socketType);
}

int socketLocalClient(const char* path, int socketType) {
    SockAddr addr;
    int ret = addr.initFromUnixPath(path);
    if (ret < 0) {
        return ret;
    }
    return socketConnectInternal(&addr, socketType);
}
#endif  // !_WIN32

int socketTcpLoopbackServer(int port, int socketType) {
    SockAddr addr;
    int ret = addr.initFromLocalhost(port);
    if (ret < 0) {
        return ret;
    }
    return socketBindInternal(&addr, socketType);
}

int socketTcpLoopbackClient(int port, int socketType) {
    SockAddr addr;
    int ret = addr.initFromLocalhost(port);
    if (ret < 0) {
        return ret;
    }
    return socketConnectInternal(&addr, socketType);
}

int socketTcpClient(const char* hostname, int port, int socketType) {
    // TODO(digit): Implement this.
    return -ENOSYS;
}

int socketAccept(int serverSocket) {
    int ret;
    do {
        ret = ::accept(serverSocket, NULL, NULL);
    } while (ret < 0 && errno == EINTR);
    return ret;
}

}  // namespace emugl
