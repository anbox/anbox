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

#ifndef ANBOX_STREAM_SOCKET_TRANSPORT_H_
#define ANBOX_STREAM_SOCKET_TRANSPORT_H_

#include "anbox/common/fd.h"
#include "anbox/network/socket_messenger.h"
#include "anbox/runtime.h"

#include <memory>

#include <boost/asio.hpp>

namespace anbox {
namespace network {
class StreamSocketTransport {
 public:
  class Observer {
   public:
    Observer() = default;
    virtual ~Observer() = default;

    virtual void on_data_available() = 0;
    virtual void on_disconnected() = 0;

    Observer(Observer const&) = delete;
    Observer& operator=(Observer const&) = delete;
  };

  StreamSocketTransport(const std::string& socket_path,
                        const std::shared_ptr<Runtime>& rt);

  void register_observer(std::shared_ptr<Observer> const& observer_);
  void unregister_observer(std::shared_ptr<Observer> const& observer_);

  void send_message(std::vector<uint8_t> const& buffer);
  void read_next_message();

 private:
  void on_read_size(const boost::system::error_code& ec,
                    std::size_t bytes_read);

  std::shared_ptr<Observer> observer_;
  std::shared_ptr<boost::asio::local::stream_protocol::socket> socket_;
  SocketMessenger messenger_;
};
}  // namespace network
}  // namespace anbox

#endif  // STREAM_SOCKET_TRANSPORT_H_
