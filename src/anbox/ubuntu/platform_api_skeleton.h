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

#ifndef ANBOX_UBUNTU_PLATFORM_SERVER_H_
#define ANBOX_UBUNTU_PLATFORM_SERVER_H_

#include "anbox/bridge/platform_api_skeleton.h"

namespace anbox {
namespace common {
class Dispatcher;
} // namespace common
namespace bridge {
class AndroidApiStub;
} // namespace bridge
namespace ubuntu {
class PlatformApiSekeleton : public bridge::PlatformApiSkeleton {
public:
    PlatformApiSekeleton(const std::shared_ptr<bridge::PendingCallCache> &pending_calls);
    virtual ~PlatformApiSekeleton();

    void handle_notification(anbox::protobuf::bridge::Notification const *request,
                             anbox::protobuf::bridge::Void *response,
                             google::protobuf::Closure *done) override;

    void boot_finished(anbox::protobuf::bridge::Void const *request,
                       anbox::protobuf::bridge::Void *response,
                       google::protobuf::Closure *done) override;

    void on_boot_finished(const std::function<void()> &action);

private:
    std::function<void()> on_boot_finished_action_;
};
} // namespace bridge
} // namespace anbox

#endif
