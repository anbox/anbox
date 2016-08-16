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

#ifndef ANBOX_BRIDGE_PLATFORM_SERVER_H_
#define ANBOX_BRIDGE_PLATFORM_SERVER_H_

#include <memory>

namespace google {
namespace protobuf {
class Closure;
} // namespace protobuf
} // namespace google

namespace anbox {
namespace protobuf {
namespace rpc {
class Void;
} // namespace rpc
namespace bridge {
class Notification;
} // namespace bridge
} // namespace protobuf
namespace rpc {
class PendingCallCache;
} // namespace rpc
namespace bridge {
class PlatformApiSkeleton {
public:
    PlatformApiSkeleton(const std::shared_ptr<rpc::PendingCallCache> &pending_calls);
    virtual ~PlatformApiSkeleton();

    virtual void boot_finished(anbox::protobuf::rpc::Void const *request,
                               anbox::protobuf::rpc::Void *response,
                               google::protobuf::Closure *done) = 0;

    virtual void handle_notification(anbox::protobuf::bridge::Notification const *request,
                                     anbox::protobuf::rpc::Void *response,
                                     google::protobuf::Closure *done) = 0;

private:
    std::shared_ptr<rpc::PendingCallCache> pending_calls_;
};
} // namespace bridge
} // namespace anbox

#endif
