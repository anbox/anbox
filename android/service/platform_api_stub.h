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

#ifndef ANBOX_ANDROID_PLATFORM_API_STUB_H_
#define ANBOX_ANDROID_PLATFORM_API_STUB_H_

#include "anbox/common/wait_handle.h"

#include <memory>

namespace anbox {
namespace protobuf {
namespace rpc {
class Void;
} // namespace rpc
} // namespace protobuf
namespace rpc {
class Channel;
} // namespace rpc
class PlatformApiStub {
public:
    PlatformApiStub(const std::shared_ptr<rpc::Channel> &rpc_channel);

    void boot_finished();
    void update_window_state();
    void remove_window();

private:
    template<typename Response>
    struct Request {
        Request() : response(std::make_shared<Response>()), success(true) { }
        std::shared_ptr<Response> response;
        bool success;
    };

    void handle_boot_finished_response(Request<protobuf::rpc::Void> *request);
    void handle_update_window_state_response(Request<protobuf::rpc::Void> *request);
    void handle_remove_window_response(Request<protobuf::rpc::Void> *request);

    mutable std::mutex mutex_;
    common::WaitHandle boot_finished_wait_handle_;
    common::WaitHandle update_window_state_wait_handle_;
    common::WaitHandle remove_window_wait_handle_;

    std::shared_ptr<rpc::Channel> rpc_channel_;
};
} // namespace anbox

#endif
