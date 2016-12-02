/*
 * Copyright © 2012-2014 Canonical Ltd.
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

#ifndef ANBOX_NETWORK_CONNECTIONS_H_
#define ANBOX_NETWORK_CONNECTIONS_H_

#include <map>
#include <memory>
#include <mutex>

namespace anbox {
namespace network {
template <class Connection>
class Connections {
 public:
  Connections() {}
  ~Connections() { clear(); }

  void add(std::shared_ptr<Connection> const& connection) {
    std::unique_lock<std::mutex> lock(mutex);
    connections.insert({connection->id(), connection});
  }

  void remove(int id) {
    std::unique_lock<std::mutex> lock(mutex);
    connections.erase(id);
  }

  bool includes(int id) const {
    std::unique_lock<std::mutex> lock(mutex);
    return connections.find(id) != connections.end();
  }

  void clear() {
    std::unique_lock<std::mutex> lock(mutex);
    connections.clear();
  }

  size_t size() { return connections.size(); }

  std::shared_ptr<Connection> at(size_t n) { return connections.at(n); }

 private:
  Connections(Connections const&) = delete;
  Connections& operator=(Connections const&) = delete;

  std::mutex mutex;
  std::map<int, std::shared_ptr<Connection>> connections;
};
}  // namespace anbox
}  // namespace network

#endif
