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

#include "emugl/common/condition_variable.h"

#include "emugl/common/lazy_instance.h"
#include "emugl/common/mutex.h"
#include "emugl/common/pod_vector.h"

// Technical note: this is loosely based on the Chromium implementation
// of ConditionVariable. This version works on Windows XP and above and
// doesn't try to use Vista's CONDITION_VARIABLE types.

namespace emugl {

namespace {

// Helper class which implements a free list of event handles.
class WaitEventStorage {
public:
    WaitEventStorage() : mFreeHandles(), mLock() {}

    ~WaitEventStorage() {
        for (size_t n = 0; n < mFreeHandles.size(); ++n) {
            CloseHandle(mFreeHandles[n]);
        }
    }

    HANDLE alloc() {
        HANDLE handle;
        mLock.lock();
        size_t size = mFreeHandles.size();
        if (size > 0) {
            handle = mFreeHandles[size - 1U];
            mFreeHandles.remove(size - 1U);
        } else {
            handle = CreateEvent(NULL, TRUE, FALSE, NULL);
        }
        mLock.unlock();
        return handle;
    }

    void free(HANDLE h) {
        mLock.lock();
        ResetEvent(h);
        mFreeHandles.push_back(h);
        mLock.unlock();
    }

private:
    PodVector<HANDLE> mFreeHandles;
    Mutex mLock;
};

LazyInstance<WaitEventStorage> sWaitEvents = LAZY_INSTANCE_INIT;

}  // namespace

ConditionVariable::ConditionVariable() : mWaiters(), mLock() {}

ConditionVariable::~ConditionVariable() {
    mLock.lock();
    for (size_t n = 0; n < mWaiters.size(); ++n) {
        CloseHandle(mWaiters[n]);
    }
    mWaiters.resize(0U);
    mLock.unlock();
}

void ConditionVariable::wait(Mutex* userLock) {
    // Grab new waiter event handle.
    mLock.lock();
    HANDLE handle = sWaitEvents->alloc();
    mWaiters.push_back(handle);
    mLock.unlock();

    // Unlock user lock then wait for event.
    userLock->unlock();
    WaitForSingleObject(handle, INFINITE);
    // NOTE: The handle has been removed from mWaiters here,
    // see signal() below. Close/recycle the event.
    sWaitEvents->free(handle);
    userLock->lock();
}

void ConditionVariable::signal() {
    mLock.lock();
    size_t size = mWaiters.size();
    if (size > 0U) {
        // NOTE: This wakes up the thread that went to sleep most
        //       recently (LIFO) for performance reason. For better
        //       fairness, using (FIFO) would be appropriate.
        HANDLE handle = mWaiters[size - 1U];
        mWaiters.remove(size - 1U);
        SetEvent(handle);
        // NOTE: The handle will be closed/recycled by the waiter.
    } else {
        // Nothing to signal.
    }
    mLock.unlock();
}

}  // namespace emugl
