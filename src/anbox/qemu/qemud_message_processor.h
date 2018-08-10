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

#ifndef ANBOX_QEMU_QEMUD_MESSAGE_PROCESSOR_H_
#define ANBOX_QEMU_QEMUD_MESSAGE_PROCESSOR_H_

#include "anbox/network/message_processor.h"
#include "anbox/network/socket_messenger.h"

namespace anbox {
namespace qemu {
class QemudMessageProcessor : public network::MessageProcessor {
 public:
  QemudMessageProcessor(
      const std::shared_ptr<network::SocketMessenger> &messenger);
  ~QemudMessageProcessor();

  bool process_data(const std::vector<std::uint8_t> &data) override;

 protected:
  virtual void handle_command(const std::string &command) = 0;

  void send_header(const size_t &size);
  void finish_message();

  std::shared_ptr<network::SocketMessenger> messenger_;

 private:
  bool process_commands();

  std::vector<std::uint8_t> buffer_;
};
}  // namespace graphics
}  // namespace anbox

#endif
