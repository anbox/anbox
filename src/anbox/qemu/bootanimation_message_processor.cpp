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

#include "anbox/qemu//bootanimation_message_processor.h"
#include "anbox/logger.h"

#include <fstream>

namespace anbox {
namespace qemu {
BootAnimationMessageProcessor::BootAnimationMessageProcessor(
    const std::shared_ptr<network::SocketMessenger> &messenger,
    const std::string &icon_path)
    : QemudMessageProcessor(messenger), icon_path_(icon_path) {}

BootAnimationMessageProcessor::~BootAnimationMessageProcessor() {}

void BootAnimationMessageProcessor::handle_command(const std::string &command) {
  if (command == "retrieve-icon") retrieve_icon();
}

void BootAnimationMessageProcessor::retrieve_icon() {
  std::ifstream icon_file(icon_path_, std::ifstream::binary);
  std::array<char, 1024> buffer;

  while (icon_file.read(buffer.data(), buffer.size())) {
    const auto bytes_read = icon_file.gcount();
    messenger_->send(buffer.data(), bytes_read);
    DEBUG("Sending %d bytes", bytes_read);
  }
}

}  // namespace qemu
}  // namespace anbox
