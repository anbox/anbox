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

#ifndef ANBOX_CONTAINER_MANAGEMENT_API_STUB_H_
#define ANBOX_CONTAINER_MANAGEMENT_API_STUB_H_

#include "anbox/common/wait_handle.h"
#include "anbox/container/configuration.h"
#include "anbox/do_not_copy_or_move.h"

#include <memory>

namespace anbox {
namespace protobuf {
namespace rpc {
class Void;
}  // namespace rpc
}  // namespace protobuf
namespace rpc {
class Channel;
}  // namespace rpc
namespace container {
class ManagementApiStub : public DoNotCopyOrMove {
 public:
  ManagementApiStub(const std::shared_ptr<rpc::Channel> &channel);
  ~ManagementApiStub();

  void start_container(const Configuration &configuration);
  void stop_container();

 private:
  template <typename Response>
  struct Request {
    Request() : response(std::make_shared<Response>()), success(true) {}
    std::shared_ptr<Response> response;
    bool success;
    common::WaitHandle wh;
  };

  void container_started(Request<protobuf::rpc::Void> *request);
  void container_stopped(Request<protobuf::rpc::Void> *request);

  mutable std::mutex mutex_;
  std::shared_ptr<rpc::Channel> channel_;
  static const std::chrono::milliseconds stop_waiting_timeout;
};
}  // namespace container
}  // namespace anbox

#endif
