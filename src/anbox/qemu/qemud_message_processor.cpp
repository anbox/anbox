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

#include "anbox/qemu/qemud_message_processor.h"
#include "anbox/logger.h"
#include "anbox/utils.h"

#include <string.h>

namespace {
static constexpr const long header_size{4};
} // namespace

namespace anbox {
namespace qemu {
QemudMessageProcessor::QemudMessageProcessor(
    const std::shared_ptr<network::SocketMessenger> &messenger)
    : messenger_(messenger) {}

QemudMessageProcessor::~QemudMessageProcessor() {}

bool QemudMessageProcessor::process_data(const std::vector<std::uint8_t> &data) {
  for (const auto &byte : data)
    buffer_.push_back(byte);

  return process_commands();
}

bool QemudMessageProcessor::process_commands() {
  while (true) {
    if (buffer_.size() < header_size)
      break;

    char header[header_size] = {0};
    ::memcpy(header, buffer_.data(), header_size);

    unsigned int body_size = 0;
    ::sscanf(header, "%04x", &body_size);

    // Double check that we have enough data to ready the whole body. If
    // not we have to wait until we have everything.
    size_t total_size = header_size + body_size;
    if (buffer_.size() < total_size)
      break;

    std::string command;
    // Make sure we only copy as much bytes as we have to and not more
    command.insert(0,
                   reinterpret_cast<const char *>(buffer_.data()) + header_size,
                   body_size);

    handle_command(command);

    const auto consumed = header_size + body_size;
    buffer_.erase(buffer_.begin(), buffer_.begin() + consumed);

    const auto remaining = buffer_.size() - consumed;
    if (remaining <= 0)
      break;
  }

  return true;
}

void QemudMessageProcessor::send_header(const size_t &size) {
  char header[header_size + 1];
  std::snprintf(header, header_size + 1, "%04zx", size);
  messenger_->send(header, header_size);
}

void QemudMessageProcessor::finish_message() {
  // Send terminating NULL byte
  messenger_->send(static_cast<const char *>(""), 1);
}
}  // namespace qemu
}  // namespace anbox
