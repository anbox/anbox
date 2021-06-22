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

#ifndef ANBOX_QEMU_CAMERA_MESSAGE_PROCESSOR_H_
#define ANBOX_QEMU_CAMERA_MESSAGE_PROCESSOR_H_

#include "anbox/network/message_processor.h"
#include "anbox/network/socket_messenger.h"

namespace anbox {
namespace qemu {
class CameraMessageProcessor : public network::MessageProcessor {
 public:
  CameraMessageProcessor(
      const std::shared_ptr<network::SocketMessenger> &messenger,
      const std::string &params);
  ~CameraMessageProcessor();

  bool process_data(const std::vector<std::uint8_t> &data) override;

 private:
  class ConnectionParameters;

  void process_commands();

  void handle_command(const std::string &command);
  
  // Command handlers
  void connect(const std::string &param);
  void disconnect(const std::string &param);
  void start(const std::string &param);
  void stop(const std::string &param);
  void frame(const std::string &param);
  void list();

  void query_reply(bool ok_ko, const void* extra, size_t extra_size);

  void reply_ok();
  void reply_ok(const std::string &result_str);
  void reply_ko(const std::string &result_str);

  void send_header(const size_t &size);
  void send_bytes(const char *msg, size_t size);

  std::shared_ptr<ConnectionParameters> connection_data_;
  std::shared_ptr<network::SocketMessenger> messenger_;
  std::vector<std::uint8_t> buffer_;
};
}  // namespace qemu
}  // namespace anbox

#endif
