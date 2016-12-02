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

#ifndef ANBOX_RPC_CHANNEL_H_
#define ANBOX_RPC_CHANNEL_H_

#include <atomic>
#include <memory>
#include <mutex>

namespace google {
namespace protobuf {
class Closure;
class MessageLite;
}  // namespace protobuf
}  // namespace google

namespace anbox {
namespace protobuf {
namespace rpc {
class Invocation;
}  // namespace rpc
}  // namespace protobuf
namespace network {
class MessageSender;
}  // namespace network
namespace rpc {
class PendingCallCache;
class Channel {
 public:
  Channel(const std::shared_ptr<PendingCallCache> &pending_calls,
          const std::shared_ptr<network::MessageSender> &sender);
  ~Channel();

  void call_method(std::string const &method_name,
                   google::protobuf::MessageLite const *parameters,
                   google::protobuf::MessageLite *response,
                   google::protobuf::Closure *complete);

  void send_event(google::protobuf::MessageLite const &event);

 private:
  protobuf::rpc::Invocation invocation_for(
      std::string const &method_name,
      google::protobuf::MessageLite const *request);
  void send_message(const std::uint8_t &type,
                    google::protobuf::MessageLite const &message);
  std::uint32_t next_id();
  void notify_disconnected();

  std::shared_ptr<PendingCallCache> pending_calls_;
  std::shared_ptr<network::MessageSender> sender_;
  std::mutex write_mutex_;
};
}  // namespace rpc
}  // namespace anbox

#endif
