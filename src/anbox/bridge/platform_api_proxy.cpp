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

#include "anbox/bridge/platform_api_proxy.h"
#include "anbox/bridge/rpc_channel.h"
#include "anbox/utils.h"
#include "anbox/config.h"
#include "anbox/logger.h"

#include "anbox_bridge.pb.h"

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace anbox {
namespace bridge {
PlatformApiProxy::PlatformApiProxy() {
}

PlatformApiProxy::~PlatformApiProxy() {
}

void PlatformApiProxy::set_rpc_channel(const std::shared_ptr<RpcChannel> &channel) {
    channel_ = channel;
}

void PlatformApiProxy::reset_rpc_channel() {
    channel_.reset();
}

void PlatformApiProxy::ensure_rpc_channel() {
    if (!channel_)
        throw std::runtime_error("No remote client connected");
}

void PlatformApiProxy::install(const std::string &path) {
    ensure_rpc_channel();

    const auto target_path = utils::string_format("%s/%s",
        config::host_share_path(), fs::path(path).filename().string());

    if (fs::exists(target_path))
        fs::remove(target_path);

    fs::copy(path, target_path);

    const auto container_path = utils::string_format("%s/%s",
        config::container_share_path(), fs::path(path).filename().string());

    auto c = std::make_shared<Request<protobuf::bridge::InstallApplication>>();
    protobuf::bridge::InstallApplication message;
    message.set_path(container_path);

    channel_->call_method("install_application",
                          &message,
                          c->response.get(),
                          google::protobuf::NewCallback(this, &PlatformApiProxy::application_installed, c.get()));
}

void PlatformApiProxy::application_installed(Request<protobuf::bridge::InstallApplication> *request) {
    DEBUG("");
}

void PlatformApiProxy::launch(const std::string &package, const std::string &activity) {
    ensure_rpc_channel();

    auto c = std::make_shared<Request<protobuf::bridge::LaunchApplication>>();
    protobuf::bridge::LaunchApplication message;
    message.set_package_name(package);
    message.set_activity(activity);

    channel_->call_method("launch_application",
                          &message,
                          c->response.get(),
                          google::protobuf::NewCallback(this, &PlatformApiProxy::application_launched, c.get()));
}

void PlatformApiProxy::application_launched(Request<protobuf::bridge::LaunchApplication> *request) {
    DEBUG("");
}

void PlatformApiProxy::set_dns_servers(const std::string &domain, const std::vector<std::string> &servers) {
    ensure_rpc_channel();

    auto c = std::make_shared<Request<protobuf::bridge::SetDnsServers>>();

    protobuf::bridge::SetDnsServers message;
    message.set_domain(domain);

    for (const auto &server : servers) {
        auto server_message = message.add_servers();
        server_message->set_address(server);
    }

    channel_->call_method("set_dns_servers",
                          &message,
                          c->response.get(),
                          google::protobuf::NewCallback(this, &PlatformApiProxy::dns_servers_set, c.get()));
}

void PlatformApiProxy::dns_servers_set(Request<protobuf::bridge::SetDnsServers> *request) {
    DEBUG("");
}
} // namespace bridge
} // namespace anbox
