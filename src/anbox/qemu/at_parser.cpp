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

#include "anbox/qemu//at_parser.h"
#include "anbox/logger.h"
#include "anbox/utils.h"

namespace anbox {
namespace qemu {
AtParser::AtParser() {}

void AtParser::register_command(const std::string &command,
                                CommandHandler handler) {
  handlers_.insert({command, handler});
}

void AtParser::process_data(std::vector<std::uint8_t> &data) {
  size_t bytes_processed = 0;
  for (size_t pos = 0; pos < data.size();) {
    const auto byte = data.at(pos);
    if (byte == '\n' || byte == '\r') {
      std::string command;
      command.insert(
          0, reinterpret_cast<const char *>(data.data()) + bytes_processed,
          pos - bytes_processed);
      bytes_processed += (pos - bytes_processed) + 1;
      processs_command(command);
    }
    pos++;
  }

  data.erase(data.begin(), data.begin() + bytes_processed);
}

void AtParser::processs_command(const std::string &command) {
  if (!utils::string_starts_with(command, "AT")) {
    WARNING("Invalid AT command: '%s'", command);
    return;
  }

  // Strip AT prefix from command
  auto real_command = command.substr(2, command.length() - 2);

  DEBUG("command: %s", real_command);

  CommandHandler handler = nullptr;
  for (const auto &iter : handlers_) {
    if (utils::string_starts_with(real_command, iter.first)) {
      handler = iter.second;
      break;
    }
  }

  if (!handler) {
    WARNING("No handler for command '%s' available", real_command);
    return;
  }

  handler(real_command);
}
}  // namespace qemu
}  // namespace anbox
