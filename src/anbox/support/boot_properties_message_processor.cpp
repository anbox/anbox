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
#include "anbox/support/boot_properties_message_processor.h"
#include "anbox/network/socket_messenger.h"
#include "anbox/network/connections.h"
#include "anbox/network/delegate_message_processor.h"

#include <string.h>

namespace ba = boost::asio;

namespace {
static constexpr const long header_size{4};
}

namespace anbox {
namespace support {
BootPropertiesMessageProcessor::BootPropertiesMessageProcessor(const std::shared_ptr<network::SocketMessenger> &messenger) :
    messenger_(messenger) {
}

BootPropertiesMessageProcessor::~BootPropertiesMessageProcessor() {
}

bool BootPropertiesMessageProcessor::process_data(const std::vector<std::uint8_t> &data) {
    for (const auto &byte : data)
        buffer_.push_back(byte);

    process_commands();

    return true;
}

void BootPropertiesMessageProcessor::process_commands() {
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

        if (command == "list")
            list_properties();
        else
            DEBUG("Unknown command '%s'", command);

        const auto consumed = header_size + body_size;
        buffer_.erase(buffer_.begin(), buffer_.begin() + consumed);

        const auto remaining = buffer_.size() - consumed;
        if (remaining <= 0)
            break;
    }
}

void BootPropertiesMessageProcessor::list_properties() {
    std::vector<std::string> properties = {
        // Simple indicator to say we're anbox and if needed things in the container
        // can adjust to this. Normally everything should detect we're qemu which
        // should be enough for the simple cases.
        "ro.anbox=1",

        "ro.hardware=goldfish",

        // Needed to let the gralloc HAL load the right implementation
        "ro.kernel.qemu.gles=1",

        // Needed from different parts of the system in order to load
        // the right implementation for qemu
        "ro.kernel.qemu=1",

        // TODO(morphis): Using HDPI here for now but should be adjusted to the device
        // we're running on.
        "qemu.sf.lcd_density=240"
    };

    for (const auto &prop : properties) {
        char header[header_size + 1];
        std::snprintf(header, header_size + 1, "%04x", prop.length());
        messenger_->send(header, header_size);
        messenger_->send(prop.c_str(), prop.length());
    }

    // Send terminating NULL byte
    messenger_->send(static_cast<const char*>(""), 1);
}
} // namespace support
} // namespace anbox
