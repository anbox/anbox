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

#include "anbox/qemu/hwcontrol_message_processor.h"
#include "anbox/logger.h"

namespace anbox {
namespace qemu {
HwControlMessageProcessor::HwControlMessageProcessor(
    const std::shared_ptr<network::SocketMessenger> &messenger)
    : QemudMessageProcessor(messenger) {}

HwControlMessageProcessor::~HwControlMessageProcessor() {}

void HwControlMessageProcessor::handle_command(const std::string &command) {
#if 0
    if (command == "power:screen_state:wake")
        DEBUG("Got screen wake command");
    else if (command == "power:screen_state:standby")
        DEBUG("Got screen standby command");
    else if (utils::string_starts_with(command, "power:light:brightness:lcd_backlight"))
        DEBUG("Got LCD backligh brightness control command");
    else
        DEBUG("Unknown command '%s'", command);
#else
  (void)command;
#endif
}
}  // namespace qemu
}  // namespace anbox
