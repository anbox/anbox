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

#ifndef ANBOX_GRAPHICS_OPENGLES_MESSAGE_PROCESSOR_H_
#define ANBOX_GRAPHICS_OPENGLES_MESSAGE_PROCESSOR_H_

#include "anbox/network/message_processor.h"
#include "anbox/network/socket_connection.h"
#include "anbox/network/socket_messenger.h"
#include "anbox/runtime.h"

#include <boost/asio.hpp>

#include <memory>
#include <mutex>

class IOStream;
class RenderThread;
class Renderer;

namespace anbox {
namespace graphics {
class OpenGlesMessageProcessor : public network::MessageProcessor {
 public:
  OpenGlesMessageProcessor(
      const std::shared_ptr<Renderer> &renderer,
      const std::shared_ptr<network::SocketMessenger> &messenger);
  ~OpenGlesMessageProcessor();

  bool process_data(const std::vector<std::uint8_t> &data) override;

 private:
  static std::mutex global_lock;

  std::shared_ptr<network::SocketMessenger> messenger_;
  std::shared_ptr<IOStream> stream_;
  std::shared_ptr<RenderThread> render_thread_;
};
}  // namespace graphics
}  // namespace anbox

#endif
