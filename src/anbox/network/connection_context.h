/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#ifndef ANBOX_NETWORK_CONNECTION_CONTEXT_H_
#define ANBOX_NETWORK_CONNECTION_CONTEXT_H_

#include <functional>
#include <memory>

namespace anbox {
namespace network {
class Connector;

class ConnectionContext {
 public:
  ConnectionContext(Connector const* connector)
      : ConnectionContext([]() {}, connector) {}
  ConnectionContext(std::function<void()> const connect_handler,
                    Connector const* connector);

  int fd_for_new_client(std::function<void()> const& connect_handler) const;
  void handle_client_connect() const { connect_handler(); }

 private:
  std::function<void()> const connect_handler;
  Connector const* const connector;
};
}  // namespace anbox
}  // namespace network

#endif
