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

#include "anbox/platform/sdl/audio_sink.h"
#include "anbox/logger.h"

#include <stdexcept>
#include <time.h>

#include <boost/throw_exception.hpp>

namespace {
const constexpr size_t max_queue_size{1};
}

namespace anbox {
namespace platform {
namespace sdl {
AudioSink::AudioSink(const std::shared_ptr<network::LocalSocketMessenger> &messenger) :
  messenger_(messenger),
  device_id_(0),
  queue_(max_queue_size) {
}

AudioSink::~AudioSink() {}

void AudioSink::on_data_requested(void *user_data, std::uint8_t *buffer, int size) {
  auto thiz = static_cast<AudioSink*>(user_data);
  thiz->read_data(buffer, size);
}

bool AudioSink::connect_audio() {
  if (device_id_ > 0)
    return true;

  SDL_memset(&spec_, 0, sizeof(spec_));
  spec_.freq = 44100;
  spec_.format = AUDIO_S16;
  spec_.channels = 2;
  spec_.samples = 1024;
  spec_.callback = &AudioSink::on_data_requested;
  spec_.userdata = this;

  device_id_ = SDL_OpenAudioDevice(nullptr, 0, &spec_, nullptr, 0);
  if (!device_id_)
    return false;

  SDL_PauseAudioDevice(device_id_, 0);

  return true;
}

void AudioSink::disconnect_audio() {
  if (device_id_ == 0)
    return;

  SDL_CloseAudioDevice(device_id_);
  device_id_ = 0;
}

void AudioSink::read_data(std::uint8_t *buffer, int size) {
  std::unique_lock<std::mutex> l(lock_);
  const auto wanted = size;
  int count = 0;
  auto dst = buffer;

  while (count < wanted) {
    if (read_buffer_left_ > 0) {
      size_t avail = std::min<size_t>(wanted - count, read_buffer_left_);
      memcpy(dst + count,
             read_buffer_.data() + (read_buffer_.size() - read_buffer_left_),
             avail);
      count += avail;
      read_buffer_left_ -= avail;
      continue;
    }

    bool blocking = (count == 0);
    auto result = -EIO;
    if (blocking)
      result = queue_.pop_locked(&read_buffer_, l);
    else
      result = queue_.try_pop_locked(&read_buffer_);

    if (result == 0) {
      read_buffer_left_ = read_buffer_.size();
      continue;
    }

    if (count > 0) break;

    return;
  }
}

void AudioSink::write_data(const std::vector<std::uint8_t> &data) {
  std::unique_lock<std::mutex> l(lock_);
  if (!connect_audio()) {
    WARNING("Audio server not connected, skipping %d bytes", data.size());
    return;
  }
  graphics::Buffer buffer{data.data(), data.data() + data.size()};
  queue_.push_locked(std::move(buffer), l);

  // Android side is waiting for "confirmation" that data arrived,
  // if sink queue is full, Android audio thread will be blocked 
  // untill there's space available in the queue
  // 
  // this acts as sychronization to time audio
  //
  // at the time of implementation, 
  // the data we send to Android is not actually used
  timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  int64_t now_us = (t.tv_sec * 1000000000LL + t.tv_nsec) / 1000;

  messenger_->send(reinterpret_cast<char*>(&now_us), sizeof(now_us));
}
} // namespace sdl
} // namespace platform
} // namespace anbox
