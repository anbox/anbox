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

#ifndef EMUGL_COMMON_SOCKETS_H
#define EMUGL_COMMON_SOCKETS_H

// A set of helper functions used to deal with sockets in a portable way.

namespace emugl {

// Disable Nagle's algorithm for socket descriptor |s|. This assumes
// that |s| is a TCP socket descriptor.
void socketTcpDisableNagle(int s);

// Return the port associated with a given IP or IPv6 socket,
// or -errno code on failure.
int socketGetPort(int s);

// Bind to a local/Unix path, and return its socket descriptor on success,
// or -errno code on failure.
int socketLocalServer(const char* path, int socketType);

// Connect to a Unix local path, and return a new socket descriptor
// on success, or -errno code on failure.
int socketLocalClient(const char* path, int socketType);

// Bind to a localhost TCP socket, and return its socket descriptor on
// success, or -errno code on failure.
int socketTcpLoopbackServer(int port, int socketType);

// Connect to a localhost TCP port, and return a new socket descriptor on
// success, or -errno code on failure.
int socketTcpLoopbackClient(int port, int socketType);

// Connect to a TCP host, and return a new socket descriptor on
// success, or -errno code on failure.
int socketTcpClient(const char* hostname, int port, int socketType);

// Accept a new connection. |serverSocket| must be a bound server socket
// descriptor. Returns new socket descriptor on success, or -errno code
// on failure.
int socketAccept(int serverSocket);

}  // namespace emugl

#endif  // EMUGL_COMMON_SOCKETS_H
