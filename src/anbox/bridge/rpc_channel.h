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

#ifndef ANBOX_BRIDGE_RPC_CHANNEL_H_
#define ANBOX_BRIDGE_RPC_CHANNEL_H_

#include <memory>
#include <atomic>
#include <mutex>

namespace google {
namespace protobuf {
class Closure;
class MessageLite;
} // namespace protobuf
} // namespace google


namespace anbox {
namespace protobuf {
namespace bridge {
class Invocation;
} // namespace bridge
} // namespace protobuf
namespace network {
class MessageSender;
} // namespace network
namespace bridge {
class PendingCallCache;
class RpcChannel {
public:
    RpcChannel(const std::shared_ptr<PendingCallCache> &pending_calls,
               const std::shared_ptr<network::MessageSender> &sender);
    ~RpcChannel();

    void call_method(std::string const& method_name,
                     google::protobuf::MessageLite const *parameters,
                     google::protobuf::MessageLite *response,
                     google::protobuf::Closure *complete);

private:
    protobuf::bridge::Invocation invocation_for(
        std::string const& method_name,
        google::protobuf::MessageLite const* request);
    void send_message(anbox::protobuf::bridge::Invocation const& invocation);
    int next_id();
    void notify_disconnected();

    std::atomic<int> next_message_id_;
    std::shared_ptr<PendingCallCache> pending_calls_;
    std::shared_ptr<network::MessageSender> sender_;
    std::mutex write_mutex_;
};
} // namespace bridge
} // namespace anbox

#endif
