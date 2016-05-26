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

#ifndef ANBOX_NETWORK_QEMU_PIPE_CONNECTION_CREATOR_H_
#define ANBOX_NETWORK_QEMU_PIPE_CONNECTION_CREATOR_H_

#include <boost/asio.hpp>

#include <memory>

#include "anbox/do_not_copy_or_move.h"
#include "anbox/runtime.h"
#include "anbox/network/connections.h"
#include "anbox/network/socket_connection.h"
#include "anbox/network/connection_creator.h"
#include "anbox/network/socket_messenger.h"

namespace anbox {
namespace network {

class QemuPipeConnectionCreator : public ConnectionCreator {
public:
    QemuPipeConnectionCreator(
            const std::shared_ptr<Runtime> &rt,
            const std::string &renderer_socket_path);
    ~QemuPipeConnectionCreator() noexcept;

    void create_connection_for(
        std::shared_ptr<boost::asio::local::stream_protocol::socket> const& socket) override;

    enum class client_type {
        invalid,
        opengles,
        qemud_boot_properties,
    };

private:
    int next_id();

    client_type identify_client(std::shared_ptr<SocketMessenger> const& messenger);
    std::shared_ptr<MessageProcessor> create_processor(
            const client_type &type, const std::shared_ptr<SocketMessenger> &messenger);

    std::shared_ptr<Runtime> runtime_;
    std::atomic<int> next_connection_id_;
    std::shared_ptr<Connections<SocketConnection>> const connections_;

    std::string renderer_socket_path_;
};
} // namespace anbox
} // namespace network

#endif
