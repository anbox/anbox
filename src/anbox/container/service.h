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

#ifndef ANBOX_CONTAINER_SERVICE_H_
#define ANBOX_CONTAINER_SERVICE_H_

#include "anbox/network/published_socket_connector.h"
#include "anbox/network/connections.h"
#include "anbox/network/socket_connection.h"
#include "anbox/network/credentials.h"
#include "anbox/container/container.h"
#include "anbox/common/dispatcher.h"
#include "anbox/runtime.h"

namespace anbox {
namespace container {
class Service : public network::Connections<network::SocketConnection>::Observer,
                public std::enable_shared_from_this<Service> {
public:
    static std::shared_ptr<Service> create(const std::shared_ptr<Runtime> &rt);

    Service(const std::shared_ptr<Runtime> &rt);
    ~Service();

    void connection_added(int id) override;
    void connection_removed(int id) override;

private:
    int next_id();
    void new_client(std::shared_ptr<boost::asio::local::stream_protocol::socket> const &socket);

    void start_container();
    void stop_container();

    std::shared_ptr<common::Dispatcher> dispatcher_;
    std::shared_ptr<network::PublishedSocketConnector> connector_;
    std::atomic<int> next_connection_id_;
    std::shared_ptr<network::Connections<network::SocketConnection>> const connections_;
    std::shared_ptr<Container> backend_;
};
} // namespace container
} // namespace anbox

#endif
