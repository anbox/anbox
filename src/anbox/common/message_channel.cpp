// Copyright 2014 The Android Open Source Project
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

#include "anbox/common/message_channel.h"

namespace anbox {
namespace common {
MessageChannelBase::MessageChannelBase(size_t capacity) : pos_(0U),
                                                          count_(0U),
                                                          capacity_(capacity),
                                                          lock_(),
                                                          can_read_(),
                                                          can_write_() {}

MessageChannelBase::~MessageChannelBase() {}

size_t MessageChannelBase::before_write() {
  std::unique_lock<std::mutex> l(lock_, std::defer_lock);
  lock_.lock();

  while (count_ >= capacity_)
    can_write_.wait(l);

  size_t result = pos_ + count_;
  if (result >= capacity_)
    result -= capacity_;

  return result;
}

void MessageChannelBase::after_write() {
  count_++;
  can_read_.notify_one();
  lock_.unlock();
}

size_t MessageChannelBase::before_read() {
  std::unique_lock<std::mutex> l(lock_, std::defer_lock);
  lock_.lock();
  while (count_ == 0)
    can_read_.wait(l);
  return pos_;
}

void MessageChannelBase::after_read() {
  if (++pos_ == capacity_)
    pos_ = 0U;
  count_--;
  can_write_.notify_one();
  lock_.unlock();
}
}  // namespace common
}  // namespace anbox
