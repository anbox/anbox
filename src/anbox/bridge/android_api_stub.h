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

#ifndef ANBOX_BRIDGE_ANDROID_API_STUB_H_
#define ANBOX_BRIDGE_ANDROID_API_STUB_H_

#include "anbox/application_manager.h"
#include "anbox/common/wait_handle.h"

#include <memory>
#include <vector>

namespace anbox {
namespace protobuf {
namespace rpc {
class Void;
} // namespace bridge
} // namespace protobuf
namespace rpc {
class Channel;
} // namespace rpc
namespace bridge {
class AndroidApiStub : public anbox::ApplicationManager {
public:
    AndroidApiStub();
    ~AndroidApiStub();

    void set_rpc_channel(const std::shared_ptr<rpc::Channel> &channel);
    void reset_rpc_channel();

    void install(const std::string &path) override;
    void launch(const std::string &package, const std::string &activity) override;

    void set_dns_servers(const std::string &domain, const std::vector<std::string> &servers);

private:
    void ensure_rpc_channel();

    template<typename Response>
    struct Request {
        Request() : response(std::make_shared<Response>()), success(true) { }
        std::shared_ptr<Response> response;
        bool success;
    };

    void application_installed(Request<protobuf::rpc::Void> *request);
    void application_launched(Request<protobuf::rpc::Void> *request);
    void dns_servers_set(Request<protobuf::rpc::Void> *request);

    mutable std::mutex mutex_;
    std::shared_ptr<rpc::Channel> channel_;
    common::WaitHandle install_wait_handle_;
    common::WaitHandle launch_wait_handle_;
    common::WaitHandle set_dns_servers_wait_handle_;
};
} // namespace bridge
} // namespace anbox

#endif
