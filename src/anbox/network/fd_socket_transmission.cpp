/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#include "anbox/network/fd_socket_transmission.h"
#include "anbox/common/variable_length_array.h"

#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <boost/exception/errinfo_errno.hpp>
#include <boost/throw_exception.hpp>
#include <stdexcept>

namespace anbox {
socket_error::socket_error(std::string const& message)
    : std::system_error(errno, std::system_category(), message) {}

socket_disconnected_error::socket_disconnected_error(std::string const& message)
    : std::system_error(errno, std::system_category(), message) {}

fd_reception_error::fd_reception_error(std::string const& message)
    : std::runtime_error(message) {}

void send_fds(Fd const& socket, std::vector<Fd> const& fds) {
  if (fds.size() > 0) {
    // We send dummy data
    struct iovec iov;
    char dummy_iov_data = 'M';
    iov.iov_base = &dummy_iov_data;
    iov.iov_len = 1;

    // Allocate space for control message
    static auto const builtin_n_fds = 5;
    static auto const builtin_cmsg_space =
        CMSG_SPACE(builtin_n_fds * sizeof(int));
    auto const fds_bytes = fds.size() * sizeof(int);
    VariableLengthArray<builtin_cmsg_space> control{CMSG_SPACE(fds_bytes)};
    // Silence valgrind uninitialized memory complaint
    memset(control.data(), 0, control.size());

    // Message to send
    struct msghdr header;
    header.msg_name = NULL;
    header.msg_namelen = 0;
    header.msg_iov = &iov;
    header.msg_iovlen = 1;
    header.msg_controllen = control.size();
    header.msg_control = control.data();
    header.msg_flags = 0;

    // Control message contains file descriptors
    struct cmsghdr* message = CMSG_FIRSTHDR(&header);
    message->cmsg_len = CMSG_LEN(fds_bytes);
    message->cmsg_level = SOL_SOCKET;
    message->cmsg_type = SCM_RIGHTS;

    int* const data = reinterpret_cast<int*>(CMSG_DATA(message));
    int i = 0;
    for (auto& fd : fds) data[i++] = fd;

    auto const sent = sendmsg(socket, &header, MSG_NOSIGNAL);
    if (sent < 0)
      BOOST_THROW_EXCEPTION(std::runtime_error("Failed to send fds: " +
                                               std::string(strerror(errno))));
  }
}

bool socket_error_is_transient(int error_code) { return (error_code == EINTR); }

void receive_data(Fd const& socket, void* buffer, size_t bytes_requested,
                  std::vector<Fd>& fds) {
  if (bytes_requested == 0)
    BOOST_THROW_EXCEPTION(std::logic_error("Attempted to receive 0 bytes"));

  size_t bytes_read{0};
  unsigned fds_read{0};
  while (bytes_read < bytes_requested) {
    // Store the data in the buffer requested
    struct iovec iov;
    iov.iov_base = static_cast<uint8_t*>(buffer) + bytes_read;
    iov.iov_len = bytes_requested - bytes_read;

    // Allocate space for control message
    static auto const builtin_n_fds = 5;
    static auto const builtin_cmsg_space =
        CMSG_SPACE(builtin_n_fds * sizeof(int));
    auto const fds_bytes = (fds.size() - fds_read) * sizeof(int);
    VariableLengthArray<builtin_cmsg_space> control{CMSG_SPACE(fds_bytes)};

    // Message to read
    struct msghdr header;
    header.msg_name = NULL;
    header.msg_namelen = 0;
    header.msg_iov = &iov;
    header.msg_iovlen = 1;
    header.msg_controllen = control.size();
    header.msg_control = control.data();
    header.msg_flags = 0;

    ssize_t const result = recvmsg(socket, &header, MSG_NOSIGNAL | MSG_WAITALL);
    if (result == 0)
      BOOST_THROW_EXCEPTION(socket_disconnected_error(
          "Failed to read message from server: server has shutdown"));
    if (result < 0) {
      if (socket_error_is_transient(errno)) continue;
      if (errno == EAGAIN) continue;
      if (errno == EPIPE)
        BOOST_THROW_EXCEPTION(
            boost::enable_error_info(
                socket_disconnected_error("Failed to read message from server"))
            << boost::errinfo_errno(errno));

      BOOST_THROW_EXCEPTION(boost::enable_error_info(socket_error(
                                "Failed to read message from server"))
                            << boost::errinfo_errno(errno));
    }

    bytes_read += result;

    // If we get a proper control message, copy the received
    // file descriptors back to the caller
    struct cmsghdr const* const cmsg = CMSG_FIRSTHDR(&header);
    if (cmsg) {
      if ((cmsg->cmsg_level == SOL_SOCKET) &&
          (cmsg->cmsg_type == SCM_CREDENTIALS))
        BOOST_THROW_EXCEPTION(
            fd_reception_error("received SCM_CREDENTIALS when expecting fd"));
      if (cmsg->cmsg_level != SOL_SOCKET || cmsg->cmsg_type != SCM_RIGHTS)
        BOOST_THROW_EXCEPTION(fd_reception_error(
            "Invalid control message for receiving file descriptors"));

      int const* const data = reinterpret_cast<int const*> CMSG_DATA(cmsg);
      ptrdiff_t const header_size = reinterpret_cast<char const*>(data) -
                                    reinterpret_cast<char const*>(cmsg);
      int const nfds = (cmsg->cmsg_len - header_size) / sizeof(int);

      // NOTE: This relies on the file descriptor cmsg being read
      // (and written) atomically.
      if (cmsg->cmsg_len > CMSG_LEN(fds_bytes) ||
          (header.msg_flags & MSG_CTRUNC)) {
        for (int i = 0; i < nfds; i++) ::close(data[i]);
        BOOST_THROW_EXCEPTION(
            std::runtime_error("Received more fds than expected"));
      }

      // We can't properly pass Fds through google::protobuf::Message,
      // which is where these get shoved.
      //
      // When we have our own RPC generator plugin and aren't using deprecated
      // Protobuf features this can go away.
      for (int i = 0; i < nfds; i++)
        fds[fds_read + i] = Fd{IntOwnedFd{data[i]}};

      fds_read += nfds;
    }
  }

  if (fds_read < fds.size()) {
    for (auto fd : fds)
      if (fd >= 0) ::close(fd);
    fds.clear();
    BOOST_THROW_EXCEPTION(
        std::runtime_error("Received fewer fds than expected"));
  }
}
}  // namespace anbox
