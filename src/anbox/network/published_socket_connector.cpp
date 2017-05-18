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

#include "anbox/network/published_socket_connector.h"
#include "anbox/network/connection_context.h"
#include "anbox/network/socket_helper.h"
#include "anbox/logger.h"

namespace anbox {
namespace network {
PublishedSocketConnector::PublishedSocketConnector(
    const std::string& socket_file, const std::shared_ptr<Runtime>& rt,
    const std::shared_ptr<ConnectionCreator<
        boost::asio::local::stream_protocol>>& connection_creator)
    : socket_file_(remove_socket_if_stale(socket_file)),
      runtime_(rt),
      connection_creator_(connection_creator),
      acceptor_(rt->service(), socket_file_) {
  start_accept();
}

PublishedSocketConnector::~PublishedSocketConnector() noexcept {}

void PublishedSocketConnector::start_accept() {
  auto socket = std::make_shared<boost::asio::local::stream_protocol::socket>(runtime_->service());

  acceptor_.async_accept(*socket,
                         [this, socket](boost::system::error_code const& err) {
                           on_new_connection(socket, err);
                         });
}

void PublishedSocketConnector::on_new_connection(std::shared_ptr<boost::asio::local::stream_protocol::socket> const& socket,
                                                 boost::system::error_code const& err) {
  if (!err)
    connection_creator_->create_connection_for(socket);

  if (err.value() == boost::asio::error::operation_aborted)
    return;

  start_accept();
}
}  // namespace network
}  // namespace anbox
