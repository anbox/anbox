/*
 * Copyright © 2012 Canonical Ltd.
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

#include "anbox/rpc/pending_call_cache.h"

#include "anbox_rpc.pb.h"

#ifdef USE_PROTOBUF_CALLBACK_HEADER
#include <google/protobuf/stubs/callback.h>
#endif

namespace anbox {
namespace rpc {
PendingCallCache::PendingCallCache() {}

void PendingCallCache::save_completion_details(
    anbox::protobuf::rpc::Invocation const& invocation,
    google::protobuf::MessageLite* response,
    google::protobuf::Closure* complete) {
  std::unique_lock<std::mutex> lock(mutex_);
  pending_calls_[invocation.id()] = PendingCall(response, complete);
}

void PendingCallCache::populate_message_for_result(
    anbox::protobuf::rpc::Result& result,
    std::function<void(google::protobuf::MessageLite*)> const& populator) {
  std::unique_lock<std::mutex> lock(mutex_);
  populator(pending_calls_.at(result.id()).response);
}

void PendingCallCache::complete_response(anbox::protobuf::rpc::Result& result) {
  PendingCall completion;

  {
    std::unique_lock<std::mutex> lock(mutex_);
    auto call = pending_calls_.find(result.id());
    if (call != pending_calls_.end()) {
      completion = call->second;
      pending_calls_.erase(call);
    }
  }

  if (completion.complete) completion.complete->Run();
}

void PendingCallCache::force_completion() {
  std::unique_lock<std::mutex> lock(mutex_);
  for (auto& call : pending_calls_) {
    auto& completion = call.second;
    completion.complete->Run();
  }

  pending_calls_.erase(pending_calls_.begin(), pending_calls_.end());
}

bool PendingCallCache::empty() const {
  std::unique_lock<std::mutex> lock(mutex_);
  return pending_calls_.empty();
}
}  // namespace rpc
}  // namespace anbox
