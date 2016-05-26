/*
* Copyright (C) 2011 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include "TcpStream.h"
#include "emugl/common/sockets.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifndef _WIN32
#include <netinet/in.h>
#include <netinet/tcp.h>
#else
#include <ws2tcpip.h>
#endif

#define LISTEN_BACKLOG 4

TcpStream::TcpStream(size_t bufSize) : SocketStream(bufSize) {}

TcpStream::TcpStream(int sock, size_t bufSize) :
    SocketStream(sock, bufSize) {
    // disable Nagle algorithm to improve bandwidth of small
    // packets which are quite common in our implementation.
    emugl::socketTcpDisableNagle(sock);
}

int TcpStream::listen(char addrstr[MAX_ADDRSTR_LEN]) {
    m_sock = emugl::socketTcpLoopbackServer(0, SOCK_STREAM);
    if (!valid())
        return int(ERR_INVALID_SOCKET);

    int port = emugl::socketGetPort(m_sock);
    if (port < 0) {
        ::close(m_sock);
        return int(ERR_INVALID_SOCKET);
    }

    snprintf(addrstr, MAX_ADDRSTR_LEN - 1, "%hu", port);
    addrstr[MAX_ADDRSTR_LEN-1] = '\0';

    return 0;
}

SocketStream * TcpStream::accept() {
    int clientSock = emugl::socketAccept(m_sock);
    if (clientSock < 0)
        return NULL;

    return new TcpStream(clientSock, m_bufsize);
}

int TcpStream::connect(const char* addr) {
    int port = atoi(addr);
    m_sock = emugl::socketTcpLoopbackClient(port, SOCK_STREAM);
    return valid() ? 0 : -1;
}

int TcpStream::connect(const char* hostname, unsigned short port)
{
    m_sock = emugl::socketTcpClient(hostname, port, SOCK_STREAM);
    return valid() ? 0 : -1;
}
