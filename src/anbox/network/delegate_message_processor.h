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

#pragma once

#include "anbox/network/message_processor.h"

#include <functional>

namespace anbox::network {
class DelegateMessageProcessor : public MessageProcessor {
 public:
  DelegateMessageProcessor(
      std::function<bool(const std::vector<std::uint8_t> &)> process_data);
  ~DelegateMessageProcessor();

  bool process_data(const std::vector<std::uint8_t> &data) override;

 private:
  std::function<bool(const std::vector<std::uint8_t> &)> process_data_;
};
}
