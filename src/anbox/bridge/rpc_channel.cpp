/*
 * Copyright © 2012 Canonical Ltd.
 *           © 2016 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include "anbox/bridge/rpc_channel.h"
#include "anbox/bridge/pending_call_cache.h"
#include "anbox/bridge/constants.h"
#include "anbox/common/variable_length_array.h"
#include "anbox/network/message_sender.h"

#include "anbox_bridge.pb.h"

namespace anbox {
namespace bridge {
RpcChannel::RpcChannel(const std::shared_ptr<PendingCallCache> &pending_calls,
                       const std::shared_ptr<network::MessageSender> &sender) :
    pending_calls_(pending_calls),
    sender_(sender) {
}

RpcChannel::~RpcChannel() {
}

void RpcChannel::call_method(std::string const& method_name,
                             google::protobuf::MessageLite const *parameters,
                             google::protobuf::MessageLite *response,
                             google::protobuf::Closure *complete) {
    auto const &invocation = invocation_for(method_name, parameters);
    pending_calls_->save_completion_details(invocation, response, complete);
    send_message(invocation);
}

protobuf::bridge::Invocation RpcChannel::invocation_for(std::string const& method_name,
                                                        google::protobuf::MessageLite const* request) {

    anbox::VariableLengthArray<2048> buffer{static_cast<size_t>(request->ByteSize())};

    request->SerializeWithCachedSizesToArray(buffer.data());

    anbox::protobuf::bridge::Invocation invoke;

    invoke.set_id(next_id());
    invoke.set_method_name(method_name);
    invoke.set_parameters(buffer.data(), buffer.size());
    invoke.set_protocol_version(1);

    return invoke;
}

void RpcChannel::send_message(anbox::protobuf::bridge::Invocation const& invocation) {
    const size_t size = invocation.ByteSize();
    const unsigned char header_bytes[header_size] = {
        static_cast<unsigned char>((size >> 8) & 0xff),
        static_cast<unsigned char>((size >> 0) & 0xff),
        MessageType::invocation,
    };

    std::vector<std::uint8_t> send_buffer(sizeof(header_bytes) + size);
    std::copy(header_bytes, header_bytes + sizeof(header_bytes), send_buffer.begin());
    invocation.SerializeToArray(send_buffer.data() + sizeof(header_bytes), size);

    try {
        std::lock_guard<std::mutex> lock(write_mutex_);
        sender_->send(reinterpret_cast<const char*>(send_buffer.data()), send_buffer.size());
    }
    catch (std::runtime_error const&) {
        notify_disconnected();
        throw;
    }
}

void RpcChannel::notify_disconnected() {
    pending_calls_->force_completion();
}

int RpcChannel::next_id() {
    return next_message_id_.fetch_add(1);
}
} // namespace bridge
} // namespace anbox
