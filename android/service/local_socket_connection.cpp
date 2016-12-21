/*
 * Copyright (C) 2016 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "android/service/local_socket_connection.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#define LOG_TAG "Anboxd"
#include <cutils/log.h>

namespace {
bool socket_error_is_transient(int error_code) {
    return (error_code == EINTR);
}
}

namespace anbox {
LocalSocketConnection::LocalSocketConnection(const std::string &path) :
    fd_(Fd::invalid) {

    struct sockaddr_un socket_address;
    memset(&socket_address, 0, sizeof(socket_address));

    socket_address.sun_family = AF_UNIX;
    memcpy(socket_address.sun_path, path.data(), path.size());

    fd_ = Fd{socket(AF_UNIX, SOCK_STREAM, 0)};
    if (connect(fd_, reinterpret_cast<sockaddr*>(&socket_address), sizeof(socket_address)) < 0)
        throw std::runtime_error("Failed to connect to server socket");
}

LocalSocketConnection::~LocalSocketConnection() {
    if (fd_ > 0)
        ::close(fd_);
}

ssize_t LocalSocketConnection::read_all(std::uint8_t *buffer, const size_t &size) {
    ssize_t bytes_read = ::recv(fd_, reinterpret_cast<void*>(buffer), size, 0);
    return bytes_read;
}

void LocalSocketConnection::send(char const* data, size_t length) {
    size_t bytes_written{0};

    while(bytes_written < length) {
        ssize_t const result = ::send(fd_,
                                      data + bytes_written,
                                      length - bytes_written,
                                      MSG_NOSIGNAL);
        if (result < 0) {
            if (socket_error_is_transient(errno))
                continue;
            else
                throw std::runtime_error("Failed to send message to server");
        }

        bytes_written += result;
    }
}

ssize_t LocalSocketConnection::send_raw(char const* data, size_t length) {
  (void)data;
  (void)length;
  return -EIO;
}
} // namespace anbox
