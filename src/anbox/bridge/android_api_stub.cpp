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

#include "anbox/bridge/android_api_stub.h"
#include "anbox/rpc/channel.h"
#include "anbox/utils.h"
#include "anbox/config.h"
#include "anbox/logger.h"

#include "anbox_rpc.pb.h"
#include "anbox_bridge.pb.h"

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace anbox {
namespace bridge {
AndroidApiStub::AndroidApiStub() {
}

AndroidApiStub::~AndroidApiStub() {
}

void AndroidApiStub::set_rpc_channel(const std::shared_ptr<rpc::Channel> &channel) {
    channel_ = channel;
}

void AndroidApiStub::reset_rpc_channel() {
    channel_.reset();
}

void AndroidApiStub::ensure_rpc_channel() {
    if (!channel_)
        throw std::runtime_error("No remote client connected");
}

void AndroidApiStub::install(const std::string &path) {
    ensure_rpc_channel();

    const auto target_path = utils::string_format("/data/anbox-share/%s", fs::path(path).filename().string());

    if (fs::exists(target_path))
        fs::remove(target_path);

    fs::copy(path, target_path);

    const auto container_path = utils::string_format("/data/anbox-share/%s", fs::path(path).filename().string());

    auto c = std::make_shared<Request<protobuf::rpc::Void>>();
    protobuf::bridge::InstallApplication message;
    message.set_path(container_path);

    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        install_wait_handle_.expect_result();
    }

    channel_->call_method("install_application",
                          &message,
                          c->response.get(),
                          google::protobuf::NewCallback(this, &AndroidApiStub::application_installed, c.get()));

    install_wait_handle_.wait_for_all();

    if (c->response->has_error())
        throw std::runtime_error(c->response->error());
}

void AndroidApiStub::application_installed(Request<protobuf::rpc::Void> *request) {
    (void) request;
    install_wait_handle_.result_received();
}

void AndroidApiStub::launch(const std::string &package, const std::string &activity) {
    ensure_rpc_channel();

    auto c = std::make_shared<Request<protobuf::rpc::Void>>();
    protobuf::bridge::LaunchApplication message;
    message.set_package_name(package);
    if (activity.length() > 0)
        message.set_activity(activity);

    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        launch_wait_handle_.expect_result();
    }

    channel_->call_method("launch_application",
                          &message,
                          c->response.get(),
                          google::protobuf::NewCallback(this, &AndroidApiStub::application_launched, c.get()));

    launch_wait_handle_.wait_for_all();

    if (c->response->has_error())
        throw std::runtime_error(c->response->error());
}

void AndroidApiStub::application_launched(Request<protobuf::rpc::Void> *request) {
    (void) request;
    launch_wait_handle_.result_received();
}

void AndroidApiStub::set_dns_servers(const std::string &domain, const std::vector<std::string> &servers) {
    ensure_rpc_channel();

    auto c = std::make_shared<Request<protobuf::rpc::Void>>();

    protobuf::bridge::SetDnsServers message;
    message.set_domain(domain);

    for (const auto &server : servers) {
        auto server_message = message.add_servers();
        server_message->set_address(server);
    }

    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        set_dns_servers_wait_handle_.expect_result();
    }

    channel_->call_method("set_dns_servers",
                          &message,
                          c->response.get(),
                          google::protobuf::NewCallback(this, &AndroidApiStub::dns_servers_set, c.get()));

    set_dns_servers_wait_handle_.wait_for_all();

    if (c->response->has_error())
        throw std::runtime_error(c->response->error());
}

void AndroidApiStub::dns_servers_set(Request<protobuf::rpc::Void> *request) {
    (void) request;
    set_dns_servers_wait_handle_.result_received();
}

void AndroidApiStub::set_focused_task(const std::int32_t &id) {
    ensure_rpc_channel();

    auto c = std::make_shared<Request<protobuf::rpc::Void>>();

    protobuf::bridge::SetFocusedTask message;
    message.set_id(id);

    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        set_focused_task_handle_.expect_result();
    }

    channel_->call_method("set_focused_task",
                          &message,
                          c->response.get(),
                          google::protobuf::NewCallback(this, &AndroidApiStub::focused_task_set, c.get()));

    set_focused_task_handle_.wait_for_all();

    if (c->response->has_error())
        throw std::runtime_error(c->response->error());
}

void AndroidApiStub::focused_task_set(Request<protobuf::rpc::Void> *request) {
    (void) request;
    set_focused_task_handle_.result_received();
}
} // namespace bridge
} // namespace anbox
