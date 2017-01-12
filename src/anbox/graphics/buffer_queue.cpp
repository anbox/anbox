// Copyright (C) 2016 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "anbox/graphics/buffer_queue.h"

namespace anbox {
namespace graphics {
BufferQueue::BufferQueue(size_t capacity)
    : capacity_(capacity), buffers_(new Buffer[capacity]) {}

int BufferQueue::try_push_locked(Buffer &&buffer) {
  if (closed_)
    return -EIO;

  if (count_ >= capacity_)
    return -EAGAIN;

  size_t pos = pos_ + count_;
  if (pos >= capacity_)
    pos -= capacity_;

  buffers_[pos] = std::move(buffer);
  if (count_++ == 0)
    can_pop_.notify_one();

  return 0;
}

int BufferQueue::push_locked(
    Buffer &&buffer, std::unique_lock<std::mutex> &lock) {
  while (count_ == capacity_) {
    if (closed_)
      return -EIO;
    can_push_.wait(lock);
  }
  return try_push_locked(std::move(buffer));
}

int BufferQueue::wait_until_not_empty_locked(std::unique_lock<std::mutex> &lock) {
  while (count_ == 0) {
    if (closed_)
      // Closed queue is empty.
      return -EIO;
    can_pop_.wait(lock);
  }

  return 0;
}

int BufferQueue::try_pop_locked(Buffer *buffer) {
  if (count_ == 0)
    return closed_ ? -EIO : -EAGAIN;

  *buffer = std::move(buffers_[pos_]);
  size_t pos = pos_ + 1;
  if (pos >= capacity_)
    pos -= capacity_;

  pos_ = pos;
  if (count_-- == capacity_)
    can_push_.notify_one();

  return 0;
}

int BufferQueue::pop_locked(
    Buffer *buffer, std::unique_lock<std::mutex> &lock) {
  while (count_ == 0) {
    if (closed_)
      // Closed queue is empty.
      return -EIO;
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
