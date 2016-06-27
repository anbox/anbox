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

#include "anbox/bridge/platform_server.h"

namespace anbox {
namespace ubuntu {
class PlatformServer : public bridge::PlatformServer {
public:
    PlatformServer(const std::shared_ptr<bridge::PendingCallCache> &pending_calls);
    virtual ~PlatformServer();

    void handle_notification(anbox::protobuf::bridge::Notification const *request,
                             anbox::protobuf::bridge::Void *response,
                             google::protobuf::Closure *done) override;
};
} // namespace bridge
} // namespace anbox

#endif
