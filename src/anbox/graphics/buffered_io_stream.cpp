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

#include "anbox/graphics/buffered_io_stream.h"
#include "anbox/logger.h"

namespace anbox {
namespace graphics {
BufferedIOStream::BufferedIOStream(
    const std::shared_ptr<anbox::network::SocketMessenger> &messenger,
    size_t buffer_size)
    : IOStream(buffer_size),
      messenger_(messenger),
      in_queue_(1024U),
      out_queue_(16U),
      worker_thread_(&BufferedIOStream::thread_main, this) {
  write_buffer_.resize_noinit(buffer_size);
}

BufferedIOStream::~BufferedIOStream() {
  forceStop();
  if (worker_thread_.joinable()) worker_thread_.join();
}

void *BufferedIOStream::allocBuffer(size_t min_size) {
  std::unique_lock<std::mutex> l(out_lock_);
  if (write_buffer_.size() < min_size) write_buffer_.resize_noinit(min_size);
  return write_buffer_.data();
}

size_t BufferedIOStream::commitBuffer(size_t size) {
  std::unique_lock<std::mutex> l(out_lock_);
  assert(size <= write_buffer_.size());
  if (write_buffer_.isAllocated()) {
    write_buffer_.resize(size);
    out_queue_.push_locked(std::move(write_buffer_), l);
  } else {
    out_queue_.push_locked(
        Buffer{write_buffer_.data(), write_buffer_.data() + size}, l);
  }
  return size;
}

const unsigned char *BufferedIOStream::read(void *buf, size_t *inout_len) {
  std::unique_lock<std::mutex> l(lock_);
  size_t wanted = *inout_len;
  size_t count = 0U;
  auto dst = static_cast<uint8_t *>(buf);
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
      result = in_queue_.pop_locked(&read_buffer_, l);
    else
      result = in_queue_.try_pop_locked(&read_buffer_);

    if (result == 0) {
      read_buffer_left_ = read_buffer_.size();
      continue;
    }

    if (count > 0) break;

    // If we end up here something went wrong and we couldn't read
    // any valid data.
    return nullptr;
  }

  *inout_len = count;
  return static_cast<const unsigned char *>(buf);
}

void BufferedIOStream::forceStop() {
  std::lock_guard<std::mutex> l(lock_);
  in_queue_.close_locked();
  out_queue_.close_locked();
}

void BufferedIOStream::post_data(Buffer &&data) {
  std::unique_lock<std::mutex> l(lock_);
  in_queue_.push_locked(std::move(data), l);
}

bool BufferedIOStream::needs_data() {
  std::unique_lock<std::mutex> l(lock_);
  return !in_queue_.can_pop_locked();
}

void BufferedIOStream::thread_main() {
  while (true) {
    std::unique_lock<std::mutex> l(out_lock_);

    Buffer buffer;
    const auto result = out_queue_.pop_locked(&buffer, l);
    if (result != 0 && result != -EAGAIN) break;

    auto bytes_left = buffer.size();
    while (bytes_left > 0) {
      const auto written = messenger_->send_raw(
          buffer.data() + (buffer.size() - bytes_left), bytes_left);
      if (written < 0) {
        if (errno != EINTR && errno != EAGAIN) {
          ERROR("Failed to write data: %s", std::strerror(errno));
          break;
        }
        // Socket is busy, lets try again
      } else
        bytes_left -= written;
    }
  }
}
}  // namespace graphics
}  // namespace anbox
