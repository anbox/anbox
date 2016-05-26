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

#include "anbox/logger.h"
#include "anbox/support/hwcontrol_message_processor.h"
#include "anbox/network/socket_messenger.h"
#include "anbox/network/connections.h"

#include <string.h>

namespace ba = boost::asio;

namespace {
static constexpr const long header_size{4};
}

namespace anbox {
namespace support {
HwControlMessageProcessor::HwControlMessageProcessor(const std::shared_ptr<network::SocketMessenger> &messenger) :
    messenger_(messenger) {
}

HwControlMessageProcessor::~HwControlMessageProcessor() {
}

bool HwControlMessageProcessor::process_data(const std::vector<std::uint8_t> &data) {
    for (const auto &byte : data)
        buffer_.push_back(byte);

    process_commands();

    return true;
}

void HwControlMessageProcessor::process_commands() {
    while (true) {
        if (buffer_.size() < header_size)
            break;

        char header[header_size] = { 0 };
        ::memcpy(header, buffer_.data(), header_size);

        unsigned int body_size = 0;
        ::sscanf(header, "%04x", &body_size);
        if (body_size != buffer_.size() - header_size)
            break;

        std::string command;
        // Make sure we only copy as much bytes as we have to and not more
        command.insert(0, reinterpret_cast<const char*>(buffer_.data()) + header_size, body_size);

        if (command == "power:screen_state:wake")
            DEBUG("Got screen wake command");
        else if (command == "power:screen_state:standby")
            DEBUG("Got screen standby command");
        else
            DEBUG("Unknown command '%s'", command);

        const auto consumed = header_size + body_size;
        buffer_.erase(buffer_.begin(), buffer_.begin() + consumed);

        const auto remaining = buffer_.size() - consumed;
        if (remaining <= 0)
            break;
    }
}
} // namespace support
} // namespace anbox
