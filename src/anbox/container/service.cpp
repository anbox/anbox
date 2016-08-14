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

#include "anbox/container/service.h"
#include "anbox/network/delegate_connection_creator.h"
#include "anbox/network/delegate_message_processor.h"
#include "anbox/network/socket_messenger.h"
#include "anbox/qemu/null_message_processor.h"
#include "anbox/container/lxc_container.h"
#include "anbox/config.h"
#include "anbox/logger.h"

namespace anbox {
namespace container {
std::shared_ptr<Service> Service::create(const std::shared_ptr<Runtime> &rt) {
    auto sp = std::make_shared<Service>(rt);

    auto delegate_connector = std::make_shared<network::DelegateConnectionCreator>(
            [sp](std::shared_ptr<boost::asio::local::stream_protocol::socket> const &socket) {
        sp->new_client(socket);
    });

    sp->connector_ = std::make_shared<network::PublishedSocketConnector>(
                config::container_socket_path(), rt, delegate_connector);

    // Make sure others can connect to our socket
    ::chmod(config::container_socket_path().c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

    sp->connections_->set_observer(sp);

    DEBUG("Everything setup. Waiting for incoming connections.");

    return sp;
}

Service::Service(const std::shared_ptr<Runtime> &rt) :
    dispatcher_(anbox::common::create_dispatcher_for_runtime(rt)),
    next_connection_id_(0),
    connections_(std::make_shared<network::Connections<network::SocketConnection>>()) {
}

Service::~Service() {
}

void Service::connection_added(int id) {
}

void Service::connection_removed(int id) {
    stop_container();
}

void Service::start_container() {
    if (backend_)
        BOOST_THROW_EXCEPTION(std::runtime_error("Container is already running"));

    backend_ = std::make_shared<LxcContainer>();
    backend_->start();
}

void Service::stop_container() {
    if (not backend_)
        return;

    backend_->stop();
    backend_.reset();
}

int Service::next_id() {
    return next_connection_id_++;
}

void Service::new_client(std::shared_ptr<boost::asio::local::stream_protocol::socket> const &socket) {
    if (connections_->size() >= 1) {
        socket->close();
        return;
    }

    auto const messenger = std::make_shared<network::SocketMessenger>(socket);
    auto const& connection = std::make_shared<network::SocketConnection>(
                messenger, messenger, next_id(), connections_,
                std::make_shared<qemu::NullMessageProcessor>());

    connections_->add(connection);
    connection->read_next_message();

    // To get access to the sockets the application manager
    // services creates we need to make sure we're running
    // against the right runtime directory here. Its a bit
    // hacky but works for now.
    auto creds = messenger->creds();
    setenv("XDG_RUNTIME_DIR", utils::string_format("/run/user/%d", creds.uid()).c_str(), 1);

    DEBUG("Got connection from pid %d", creds.pid());

    start_container();
}
} // namespace container
} // namespace anbox
