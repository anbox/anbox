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

#include "anbox/graphics/opengles_message_processor.h"
#include "anbox/graphics/emugl/RenderThread.h"
#include "anbox/logger.h"
#include "anbox/network/connections.h"
#include "anbox/network/delegate_message_processor.h"

#include <condition_variable>
#include <functional>
#include <queue>

namespace anbox {
namespace graphics {
std::mutex OpenGlesMessageProcessor::global_lock{};

OpenGlesMessageProcessor::OpenGlesMessageProcessor(
    const std::shared_ptr<Renderer> &renderer,
    const std::shared_ptr<network::SocketMessenger> &messenger)
    : messenger_(messenger),
      stream_(std::make_shared<BufferedIOStream>(messenger_)) {
  // We have to read the client flags first before we can continue
  // processing the actual commands
  unsigned int client_flags = 0;
  auto err = messenger_->receive_msg(
      boost::asio::buffer(&client_flags, sizeof(unsigned int)));
  if (err) ERROR("%s", err.message());

  render_thread_.reset(RenderThread::create(renderer, stream_.get(), std::ref(global_lock)));
  if (!render_thread_->start())
    BOOST_THROW_EXCEPTION(
        std::runtime_error("Failed to start renderer thread"));
}

OpenGlesMessageProcessor::~OpenGlesMessageProcessor() {
  render_thread_->forceStop();
  render_thread_->wait(nullptr);
}

bool OpenGlesMessageProcessor::process_data(
    Buffer &&data) {
  auto stream = std::static_pointer_cast<BufferedIOStream>(stream_);
  stream->post_data(std::move(data));
  return true;
}
}  // namespace graphics
}  // namespace anbox
