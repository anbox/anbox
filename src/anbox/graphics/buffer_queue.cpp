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

#include "anbox/graphics/buffer_queue.h"

namespace anbox {
namespace graphics {
BufferQueue::BufferQueue(size_t capacity)
    : capacity_(capacity), buffers_(new Buffer[capacity]) {}

BufferQueue::Result BufferQueue::try_push_locked(Buffer &&buffer) {
  if (closed_) {
    return Result::Error;
  }
  if (count_ >= capacity_) {
    return Result::TryAgain;
  }
  size_t pos = pos_ + count_;
  if (pos >= capacity_) {
    pos -= capacity_;
  }
  buffers_[pos] = std::move(buffer);
  if (count_++ == 0) {
    can_pop_.notify_one();
  }
  return Result::Ok;
}

BufferQueue::Result BufferQueue::push_locked(
    Buffer &&buffer, std::unique_lock<std::mutex> &lock) {
  while (count_ == capacity_) {
    if (closed_) {
      return Result::Error;
    }
    can_push_.wait(lock);
  }
  return try_push_locked(std::move(buffer));
}

BufferQueue::Result BufferQueue::try_pop_locked(Buffer *buffer) {
  if (count_ == 0) {
    return closed_ ? Result::Error : Result::TryAgain;
  }
  *buffer = std::move(buffers_[pos_]);
  size_t pos = pos_ + 1;
  if (pos >= capacity_) {
    pos -= capacity_;
  }
  pos_ = pos;
  if (count_-- == capacity_) {
    can_push_.notify_one();
  }
  return Result::Ok;
}

BufferQueue::Result BufferQueue::pop_locked(
    Buffer *buffer, std::unique_lock<std::mutex> &lock) {
  while (count_ == 0) {
    if (closed_) {
      // Closed queue is empty.
      return Result::Error;
    }
    can_pop_.wait(lock);
  }
  return try_pop_locked(buffer);
}

// Close the queue, it is no longer possible to push new items
// to it (i.e. push() will always return Result::Error), or to
// read from an empty queue (i.e. pop() will always return
// Result::Error once the queue becomes empty).
void BufferQueue::close_locked() {
  closed_ = true;

  // Wake any potential waiters.
  if (count_ == capacity_) {
    can_push_.notify_all();
  }
  if (count_ == 0) {
    can_pop_.notify_all();
  }
}
}  // namespace graphics
}  // namespace anbox
