/*
 * Copyright (C) 2017 Simon Fels <morphis@gravedo.de>
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

#ifndef ANBOX_PLATFORM_SDL_AUDIO_SINK_H_
#define ANBOX_PLATFORM_SDL_AUDIO_SINK_H_

#include "anbox/audio/sink.h"
#include "anbox/graphics/buffer_queue.h"
#include "anbox/platform/sdl/sdl_wrapper.h"
#include "anbox/network/local_socket_messenger.h"

#include <thread>

namespace anbox {
namespace platform {
namespace sdl {
class AudioSink : public audio::Sink {
 public:
  AudioSink(const std::shared_ptr<network::LocalSocketMessenger> &messenger);
  ~AudioSink();

  void write_data(const std::vector<std::uint8_t> &data) override;

 private:
  bool connect_audio();
  void disconnect_audio();
  void read_data(std::uint8_t *buffer, int size);

  static void on_data_requested(void *user_data, std::uint8_t *buffer, int size);
  
  std::shared_ptr<anbox::network::LocalSocketMessenger> messenger_;
  std::mutex lock_;
  SDL_AudioSpec spec_;
  SDL_AudioDeviceID device_id_;
  graphics::BufferQueue queue_;
  graphics::Buffer read_buffer_;
  size_t read_buffer_left_ = 0;
};
} // namespace sdl
} // namespace platform
} // namespace anbox

#endif
