// Copyright (C) 2014 The Android Open Source Project
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

#include "emugl/common/smart_ptr.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace emugl {

// Thread-safe atomic reference-counting type.
class RefCount {
public:
    RefCount() : mCount(1) {}
    ~RefCount() {}

    // Technically not thread-safe, use only for testing.
    int count() const { return (int)mCount; }

    void increment() {
#ifdef _WIN32
        InterlockedIncrement(&mCount);
#else
        __sync_fetch_and_add(&mCount, 1);
#endif
    }

    bool decrement() {
#ifdef _WIN32
        return InterlockedDecrement(&mCount) == 0;
#else
        return __sync_add_and_fetch(&mCount, -1) == 0;
#endif
    }

private:
#ifdef _WIN32
    LONG mCount;
#else
    int mCount;
#endif
};


// SmartPtrBase implementation.

SmartPtrBase::SmartPtrBase(void* ptr) : mPtr(ptr), mRefCount(NULL) {
    if (mPtr)
        mRefCount = new RefCount();
}


SmartPtrBase::SmartPtrBase(const SmartPtrBase& other)
        : mPtr(other.mPtr), mRefCount(other.mRefCount) {
    if (mRefCount)
        mRefCount->increment();
}


int SmartPtrBase::getRefCount() const {
    return mRefCount ? mRefCount->count() : 0;
}


void SmartPtrBase::addRef() {
    if (mRefCount)
        mRefCount->increment();
}


void* SmartPtrBase::copyFrom(const SmartPtrBase& other) {
    void* old_ptr = release();

    mPtr = other.mPtr;
    mRefCount = other.mRefCount;
    if (mRefCount)
        mRefCount->increment();

    return old_ptr;
}


void* SmartPtrBase::release() {
    void* old_ptr = mPtr;
    RefCount* old_refcount = mRefCount;

    if (old_refcount) {
        mPtr = NULL;
        mRefCount = NULL;

        if (old_refcount->decrement()) {
          delete old_refcount;
          return old_ptr;
        }
    }

    return NULL;
}

}  // namespace emugl
