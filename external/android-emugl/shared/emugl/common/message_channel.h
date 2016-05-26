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

#ifndef EMUGL_COMMON_MESSAGE_CHANNEL_H
#define EMUGL_COMMON_MESSAGE_CHANNEL_H

#include "emugl/common/condition_variable.h"
#include "emugl/common/mutex.h"

#include <stddef.h>

namespace emugl {

// Base non-templated class used to reduce the amount of template
// specialization.
class MessageChannelBase {
public:
    // Constructor. |capacity| is the buffer capacity in messages.
    MessageChannelBase(size_t capacity);

    // Destructor.
    ~MessageChannelBase();

protected:
    // Call this method in the sender thread before writing a new message.
    // This returns the position of the available slot in the message array
    // where to copy the new fixed-size message. After the copy, call
    // afterWrite().
    size_t beforeWrite();

    // To be called after beforeWrite() and copying a new fixed-size message
    // into the array. This signal the receiver thread that there is a new
    // incoming message.
    void afterWrite();

    // Call this method in the receiver thread before reading a new message.
    // This returns the position in the message array where the new message
    // can be read. Caller must process the message, then call afterRead().
    size_t beforeRead();

    // To be called in the receiver thread after beforeRead() and processing
    // the corresponding message.
    void afterRead();

private:
    size_t mPos;
    size_t mCount;
    size_t mCapacity;
    Mutex mLock;
    ConditionVariable mCanRead;
    ConditionVariable mCanWrite;
};

// Helper class used to implement an uni-directional IPC channel between
// two threads. The channel can be used to send fixed-size messages of type
// |T|, with an internal buffer size of |CAPACITY| items. All calls are
// blocking.
//
// Usage is pretty straightforward:
//
//   - From the sender thread, call send(msg);
//   - From the receiver thread, call receive(&msg);
//
template <typename T, size_t CAPACITY>
class MessageChannel : public MessageChannelBase {
public:
    MessageChannel() : MessageChannelBase(CAPACITY) {}

    void send(const T& msg) {
        size_t pos = beforeWrite();
        mItems[pos] = msg;
        afterWrite();
    }

    void receive(T* msg) {
        size_t pos = beforeRead();
        *msg = mItems[pos];
        afterRead();
    }

private:
    T mItems[CAPACITY];
};

}  // namespace emugl

#endif  // EMUGL_COMMON_MESSAGE_CHANNEL_H
