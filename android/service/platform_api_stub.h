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
#include <vector>
#include <string>

namespace anbox {
namespace protobuf {
namespace rpc {
class Void;
} // namespace rpc
namespace bridge {
class ClipboardData;
} // namespace bridge
} // namespace protobuf
namespace rpc {
class Channel;
} // namespace rpc
class PlatformApiStub {
public:
    PlatformApiStub(const std::shared_ptr<rpc::Channel> &rpc_channel);

    void boot_finished();

    struct WindowStateUpdate {
        struct Window {
            int display_id;
            bool has_surface;
            std::string package_name;
            struct Frame {
                int left;
                int top;
                int right;
                int bottom;
            };
            Frame frame;
            int task_id;
            int stack_id;
        };
        std::vector<Window> updated_windows;
        std::vector<Window> removed_windows;
    };

    void update_window_state(const WindowStateUpdate &state);

    struct ApplicationListUpdate {
        struct Application {
            std::string name;
            std::string package;
            struct Intent {
                std::string action;
                std::string uri;
                std::string type;
                std::string package;
                std::string component;
                std::vector<std::string> categories;
            };
            Intent launch_intent;
            std::vector<int8_t> icon;
        };
        std::vector<Application> applications;
        std::vector<std::string> removed_applications;
    };

    void update_application_list(const ApplicationListUpdate &update);

    struct ClipboardData {
        std::string text;
    };

    void set_clipboard_data(const ClipboardData &data);
    ClipboardData get_clipboard_data();

private:
    template<typename Response>
    struct Request {
        Request() : response(std::make_shared<Response>()), success(true) { }
        std::shared_ptr<Response> response;
        bool success;
        common::WaitHandle wh;
    };

    void on_clipboard_data_set(Request<protobuf::rpc::Void> *request);
    void on_clipboard_data_get(Request<protobuf::bridge::ClipboardData> *request);

    mutable std::mutex mutex_;
    std::shared_ptr<rpc::Channel> rpc_channel_;

    ClipboardData received_clipboard_data_;
};
} // namespace anbox

#endif
