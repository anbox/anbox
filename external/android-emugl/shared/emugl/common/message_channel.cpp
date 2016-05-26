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

#include "emugl/common/message_channel.h"

namespace emugl {

MessageChannelBase::MessageChannelBase(size_t capacity) :
        mPos(0U),
        mCount(0U),
        mCapacity(capacity),
        mLock(),
        mCanRead(),
        mCanWrite() {}

MessageChannelBase::~MessageChannelBase() {}

size_t MessageChannelBase::beforeWrite() {
    mLock.lock();
    while (mCount >= mCapacity) {
        mCanWrite.wait(&mLock);
    }
    size_t result = mPos + mCount;
    if (result >= mCapacity) {
        result -= mCapacity;
    }
    return result;
}

void MessageChannelBase::afterWrite() {
    mCount++;
    mCanRead.signal();
    mLock.unlock();
}

size_t MessageChannelBase::beforeRead() {
    mLock.lock();
    while (mCount == 0) {
        mCanRead.wait(&mLock);
    }
    return mPos;
}

void MessageChannelBase::afterRead() {
    if (++mPos == mCapacity) {
        mPos = 0U;
    }
    mCount--;
    mCanWrite.signal();
    mLock.unlock();
}

}  // namespace emugl
