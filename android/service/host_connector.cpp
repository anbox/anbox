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

#include "android/service/host_connector.h"
#include "android/service/local_socket_connection.h"
#include "android/service/message_processor.h"
#include "android/service/platform_api.h"

#include <functional>
#include <array>

namespace anbox {
namespace android {
HostConnector::HostConnector() :
    socket_(std::make_shared<LocalSocketConnection>("/dev/anbox_bridge")),
    pending_calls_(std::make_shared<bridge::PendingCallCache>()),
    platform_api_(std::make_shared<PlatformApi>()),
    message_processor_(std::make_shared<MessageProcessor>(socket_, pending_calls_, platform_api_)),
    running_(false) {
}

HostConnector::~HostConnector() {
}

void HostConnector::start() {
    if (running_)
        return;

    running_.exchange(true);
    thread_ = std::thread(std::bind(&HostConnector::main_loop, this));
}

void HostConnector::stop() {
    if (!running_.exchange(false))
        return;

    thread_.join();
}

void HostConnector::main_loop() {
    while (running_) {
        std::array<std::uint8_t, 8192> buffer;
        const auto bytes_read = socket_->read_all(buffer.data(), buffer.size());
        if (bytes_read == 0)
            break;

        // MessageProcessor wants an vector so give it what it wants until
        // we refactor this.
        std::vector<std::uint8_t> data;
        for (auto n = 0; n < bytes_read; n++)
            data.push_back(buffer[n]);

        if (!message_processor_->process_data(data))
            break;
    }

    // FIXME notify our core that we've stopped
}
} // namespace android
} // namespace anbox
