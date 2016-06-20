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

#include "anbox/support/boot_properties_message_processor.h"
#include "anbox/logger.h"

namespace anbox {
namespace support {
BootPropertiesMessageProcessor::BootPropertiesMessageProcessor(const std::shared_ptr<network::SocketMessenger> &messenger) :
    QemudMessageProcessor(messenger) {
}

BootPropertiesMessageProcessor::~BootPropertiesMessageProcessor() {
}


void BootPropertiesMessageProcessor::handle_command(const std::string &command) {
    if (command == "list")
        list_properties();
    else
        DEBUG("Unknown command '%s'", command);
}

void BootPropertiesMessageProcessor::list_properties() {
    std::vector<std::string> properties = {
        // Simple indicator to say we're anbox and if needed things in the container
        // can adjust to this. Normally everything should detect we're qemu which
        // should be enough for the simple cases.
        "ro.anbox=1",

        // Needed for identification within the system. Normally retrieve from
        // kernel configuration which isn't possible in our case.
        "ro.hardware=goldfish",

        // Needed to let the gralloc HAL load the right implementation
        "ro.kernel.qemu.gles=1",

        // Needed from different parts of the system in order to load
        // the right implementation for qemu
        "ro.kernel.qemu=1",

        // TODO(morphis): Using HDPI here for now but should be adjusted to the device
        // we're running on.
        "qemu.sf.lcd_density=240",

        // libhwui detects that we support certain GLESv3 extensions which
        // we don't yet support in our host channel so we have to disable
        // those things here.
        "ro.hwui.use_gpu_pixel_buffers=0",

        // This will give us a virtual on screen navigation area at the bottom
        // of the screen we really need to navigate through the system.
        "qemu.hw.mainkeys=0",
    };

    for (const auto &prop : properties) {
        send_header(prop.length());
        messenger_->send(prop.c_str(), prop.length());
    }

    finish_message();
}
} // namespace support
} // namespace anbox
