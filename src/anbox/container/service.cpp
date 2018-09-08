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
#include "anbox/system_configuration.h"
#include "anbox/container/lxc_container.h"
#include "anbox/container/management_api_message_processor.h"
#include "anbox/container/management_api_skeleton.h"
#include "anbox/logger.h"
#include "anbox/network/delegate_connection_creator.h"
#include "anbox/network/delegate_message_processor.h"
#include "anbox/network/local_socket_messenger.h"
#include "anbox/qemu/null_message_processor.h"
#include "anbox/rpc/channel.h"
#include "anbox/rpc/pending_call_cache.h"

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace anbox {
namespace container {
std::shared_ptr<Service> Service::create(const std::shared_ptr<Runtime> &rt, const Configuration &config) {
  auto sp = std::shared_ptr<Service>(new Service(rt, config));

  auto wp = std::weak_ptr<Service>(sp);
  auto delegate_connector = std::make_shared<network::DelegateConnectionCreator<boost::asio::local::stream_protocol>>(
      [wp](std::shared_ptr<boost::asio::local::stream_protocol::socket> const &socket) {
        if (auto service = wp.lock())
          service->new_client(socket);
  });

  const auto container_socket_path = SystemConfiguration::instance().container_socket_path();
  const auto socket_parent_path = fs::path(container_socket_path).parent_path();
  if (!fs::exists(socket_parent_path))
    fs::create_directories(socket_parent_path);

  sp->connector_ = std::make_shared<network::PublishedSocketConnector>(container_socket_path, rt, delegate_connector);

  // Make sure others can connect to our socket
  ::chmod(container_socket_path.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

  DEBUG("Everything setup. Waiting for incoming connections.");

  return sp;
}

Service::Service(const std::shared_ptr<Runtime> &rt, const Configuration &config)
    : dispatcher_(anbox::common::create_dispatcher_for_runtime(rt)),
      next_connection_id_(0),
      connections_(std::make_shared<network::Connections<network::SocketConnection>>()),
      config_(config) {
}

Service::~Service() {
  connections_->clear();
}

int Service::next_id() { return next_connection_id_++; }

void Service::new_client(std::shared_ptr<boost::asio::local::stream_protocol::socket> const
        &socket) {
  if (connections_->size() >= 1) {
    socket->close();
    return;
  }

  auto const messenger = std::make_shared<network::LocalSocketMessenger>(socket);

  DEBUG("Got connection from pid %d", messenger->creds().pid());

  auto pending_calls = std::make_shared<rpc::PendingCallCache>();
  auto rpc_channel = std::make_shared<rpc::Channel>(pending_calls, messenger);
  auto server = std::make_shared<container::ManagementApiSkeleton>(
      pending_calls, std::make_shared<LxcContainer>(config_.privileged,
                                                    config_.rootfs_overlay,
                                                    config_.container_network_address,
                                                    config_.container_network_gateway,
                                                    config_.container_network_dns_servers,
                                                    messenger->creds()));
  auto processor = std::make_shared<container::ManagementApiMessageProcessor>(
      messenger, pending_calls, server);

  auto const &connection = std::make_shared<network::SocketConnection>(
      messenger, messenger, next_id(), connections_, processor);
  connection->set_name("container-service");

  connections_->add(connection);
  connection->read_next_message();
}
}  // namespace container
}  // namespace anbox
