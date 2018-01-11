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
#include "anbox/logger.h"
#include <string>

namespace anbox {
namespace qemu {
BootPropertiesMessageProcessor::BootPropertiesMessageProcessor(
    const std::shared_ptr<network::SocketMessenger> &messenger)
    : QemudMessageProcessor(messenger) {}

BootPropertiesMessageProcessor::~BootPropertiesMessageProcessor() {}

void BootPropertiesMessageProcessor::handle_command(
    const std::string &command) {
  if (command == "list") list_properties();
}

void BootPropertiesMessageProcessor::list_properties() {
  // For ubuntu-touch we use GRID_UNIT_PX to calculate lcd_density
  int density = static_cast<int>(graphics::DensityType::medium);
  if (utils::get_env_value("GRID_UNIT_PX").length() > 0) {
    auto grid_unit_px = std::stoi(utils::get_env_value("GRID_UNIT_PX"));
    density = grid_unit_px * 20;
    DEBUG("Using grid_units %d", grid_unit_px);
  }
  DEBUG("density %d", density);
  std::vector<std::string> properties = {
      // TODO(morphis): Using HDPI here for now but should be adjusted to the
      // device
      // we're running on.
      utils::string_format("ro.sf.lcd_density=%d", density),
  };

  for (const auto &prop : properties) {
    send_header(prop.length());
    messenger_->send(prop.c_str(), prop.length());
  }

  finish_message();
}
}  // namespace qemu
}  // namespace anbox
