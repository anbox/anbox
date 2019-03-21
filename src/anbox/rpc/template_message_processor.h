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

#ifndef ANBOX_RPC_TEMPLATE_MESSAGE_PROCESSOR_H_
#define ANBOX_RPC_TEMPLATE_MESSAGE_PROCESSOR_H_

#include <google/protobuf/stubs/common.h>
#ifdef USE_PROTOBUF_CALLBACK_HEADER
#include <google/protobuf/stubs/callback.h>
#endif

#include "anbox/rpc/message_processor.h"

#include "anbox_rpc.pb.h"

namespace anbox {
namespace rpc {
// Utility metafunction result_ptr_t<> allows invoke() to pick the right
// send_response() overload. The base template resolves to the prototype
// "send_response(::google::protobuf::uint32 id, ::google::protobuf::Message*
// response)"
// Client code may specialize result_ptr_t to resolve to another overload.
template <typename ResultType>
struct result_ptr_t {
  typedef ::google::protobuf::MessageLite* type;
};

// Boiler plate for unpacking a parameter message, invoking a server function,
// and
// sending the result message. Assumes the existence of Self::send_response().
template <class Self, class Bridge, class BridgeX, class ParameterMessage,
          class ResultMessage>
void invoke(Self* self, Bridge* rpc,
            void (BridgeX::*function)(ParameterMessage const* request,
                                      ResultMessage* response,
                                      ::google::protobuf::Closure* done),
            Invocation const& invocation) {
  ParameterMessage parameter_message;
  if (!parameter_message.ParseFromString(invocation.parameters()))
    throw std::runtime_error("Failed to parse message parameters!");
  ResultMessage result_message;

  try {
    std::unique_ptr<google::protobuf::Closure> callback(
        google::protobuf::NewPermanentCallback<
            Self, ::google::protobuf::uint32,
            typename result_ptr_t<ResultMessage>::type>(
            self, &Self::send_response, invocation.id(), &result_message));

    (rpc->*function)(&parameter_message, &result_message, callback.get());
  } catch (std::exception const& x) {
    result_message.set_error(std::string("Error processing request: ") +
                             x.what());
    self->send_response(invocation.id(), &result_message);
  }
}
}  // namespace rpc
}  // namespace anbox

#endif
