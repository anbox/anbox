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

#include "anbox/qemu/null_message_processor.h"
#include "anbox/logger.h"
#include "anbox/utils.h"

#include <string.h>

namespace anbox {
namespace qemu {
NullMessageProcessor::NullMessageProcessor() {}

NullMessageProcessor::~NullMessageProcessor() {}

bool NullMessageProcessor::process_data(const std::vector<std::uint8_t> &data) {
  (void)data;
  return true;
}
}  // namespace qemu
}  // namespace anbox
