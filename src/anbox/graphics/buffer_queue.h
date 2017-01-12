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

#ifndef ANBOX_GRAPHICS_BUFFER_QUEUE_H_
#define ANBOX_GRAPHICS_BUFFER_QUEUE_H_

#include "anbox/common/small_vector.h"

#include <condition_variable>
#include <memory>
#include <mutex>

namespace anbox {
namespace graphics {
using Buffer = anbox::common::SmallFixedVector<char, 512>;

class BufferQueue {
 public:
  BufferQueue(size_t capacity);

  bool can_push_locked() const { return !closed_ && (count_ < capacity_); }
  bool can_pop_locked() const { return count_ > 0U; }
  bool is_closed_locked() const { return closed_; }

  int wait_until_not_empty_locked(std::unique_lock<std::mutex> &lock);

  int try_push_locked(Buffer &&buffer);
  int push_locked(Buffer &&buffer, std::unique_lock<std::mutex> &lock);
  int try_pop_locked(Buffer *buffer);
  int pop_locked(Buffer *buffer, std::unique_lock<std::mutex> &lock);
  void close_locked();

 private:
  size_t capacity_ = 0;
  size_t pos_ = 0;
  size_t count_ = 0;
  bool closed_ = false;
  std::unique_ptr<Buffer[]> buffers_;

  std::condition_variable can_push_;
  std::condition_variable can_pop_;
};

}  // namespace graphics
}  // namespace anbox

#endif
