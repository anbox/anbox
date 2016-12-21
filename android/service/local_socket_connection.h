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

#ifndef ANBOX_ANDROID_LOCAL_SOCKET_CONNECTION_H_
#define ANBOX_ANDROID_LOCAL_SOCKET_CONNECTION_H_

#include <string>
#include <vector>

#include "anbox/common/fd.h"
#include "anbox/network/message_sender.h"

namespace anbox {
class LocalSocketConnection : public network::MessageSender {
public:
    LocalSocketConnection(const std::string &path);
    ~LocalSocketConnection();

    ssize_t read_all(std::uint8_t *buffer, const size_t &size);
    void send(char const* data, size_t length) override;
    ssize_t send_raw(char const* data, size_t length) override;

private:
    Fd fd_;
};
} // namespace anbox

#endif
