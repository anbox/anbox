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

#ifndef ANBOX_CONTAINER_CLIENT_H_
#define ANBOX_CONTAINER_CLIENT_H_

#include "anbox/container/configuration.h"
#include "anbox/runtime.h"

namespace anbox {
namespace rpc {
class PendingCallCache;
class Channel;
class MessageProcessor;
}  // namespace rpc
namespace network {
class LocalSocketMessenger;
}  // namespace network
namespace container {
class ManagementApiStub;
class Client {
 public:
  typedef std::function<void()> TerminateCallback;

  Client(const std::shared_ptr<Runtime> &rt);
  ~Client();

  void start(const Configuration &configuration);
  void stop();

  void register_terminate_handler(const TerminateCallback &callback);

 private:
  void read_next_message();
  void on_read_size(const boost::system::error_code &ec,
                    std::size_t bytes_read);

  std::shared_ptr<network::LocalSocketMessenger> messenger_;
  std::shared_ptr<rpc::PendingCallCache> pending_calls_;
  std::shared_ptr<rpc::Channel> rpc_channel_;
  std::shared_ptr<ManagementApiStub> management_api_;
  std::shared_ptr<rpc::MessageProcessor> processor_;
  std::array<std::uint8_t, 8192> buffer_;
  TerminateCallback terminate_callback_;
};
}  // namespace container
}  // namespace anbox

#endif
