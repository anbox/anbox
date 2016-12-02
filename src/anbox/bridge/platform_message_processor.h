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

#ifndef ANBOX_BRIDGE_PLATFORM_MESSAGE_PROCESSOR_H_
#define ANBOX_BRIDGE_PLATFORM_MESSAGE_PROCESSOR_H_

#include "anbox/rpc/message_processor.h"

namespace anbox {
namespace bridge {
class PlatformApiSkeleton;
class PlatformMessageProcessor : public rpc::MessageProcessor {
 public:
  PlatformMessageProcessor(
      const std::shared_ptr<network::MessageSender> &sender,
      const std::shared_ptr<PlatformApiSkeleton> &server,
      const std::shared_ptr<rpc::PendingCallCache> &pending_calls);
  ~PlatformMessageProcessor();

  void dispatch(rpc::Invocation const &invocation) override;
  void process_event_sequence(const std::string &event) override;

 private:
  std::shared_ptr<PlatformApiSkeleton> server_;
};
}  // namespace anbox
}  // namespace network

#endif
