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
