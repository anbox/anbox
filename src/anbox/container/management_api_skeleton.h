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

#pragma once

#include <memory>

namespace google {
  namespace protobuf {
    class Closure;
  }
}

namespace anbox::protobuf::rpc {
  class Void;
}
namespace anbox::protobuf::container {
  class StartContainer;
  class StopContainer;
}

namespace anbox::rpc {
  class PendingCallCache;
}
namespace anbox::container {
class Container;
class ManagementApiSkeleton {
 public:
  ManagementApiSkeleton(
      const std::shared_ptr<Container> &container);
  ~ManagementApiSkeleton();

  void start_container(
      anbox::protobuf::container::StartContainer const *request,
      anbox::protobuf::rpc::Void *response, google::protobuf::Closure *done);

  void stop_container(
      anbox::protobuf::container::StopContainer const *request,
      anbox::protobuf::rpc::Void *response, google::protobuf::Closure *done);

 private:
  std::shared_ptr<Container> container_;
};
}

