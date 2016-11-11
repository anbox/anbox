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

#include "anbox/qemu//boot_properties_message_processor.h"
#include "anbox/graphics/density.h"
#include "anbox/utils.h"

namespace anbox {
namespace qemu {
BootPropertiesMessageProcessor::BootPropertiesMessageProcessor(const std::shared_ptr<network::SocketMessenger> &messenger) :
    QemudMessageProcessor(messenger) {
}

BootPropertiesMessageProcessor::~BootPropertiesMessageProcessor() {
}


void BootPropertiesMessageProcessor::handle_command(const std::string &command) {
    if (command == "list")
        list_properties();
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
        utils::string_format("ro.sf.lcd_density=%d", static_cast<int>(graphics::DensityType::medium)),

        // Disable on-screen virtual keys as we can use the hardware keyboard
        "qemu.hw.mainkeys=1",

        // Android has builtin detection (inside Zygote) for proper container
        // detection support
        "ro.boot.container=1",

        // Mark us as a device without telephony support (as we don't have a RIL)
        "ro.radio.noril=yes",

        // To let surfaceflinger load our hwcomposer implementation we specify
        // the correct subkey of the module here.
        "ro.hardware.hwcomposer=anbox"

        // Keep boot animation disabled as we don't need it
        "debug.sf.nobootanimation=1",
    };

    for (const auto &prop : properties) {
        send_header(prop.length());
        messenger_->send(prop.c_str(), prop.length());
    }

    finish_message();
}
} // namespace qemu
} // namespace anbox
