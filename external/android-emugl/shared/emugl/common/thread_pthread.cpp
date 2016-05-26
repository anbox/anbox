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

#include "emugl/common/thread.h"

#include "emugl/common/thread_store.h"

#include <assert.h>
#include <stdio.h>

namespace emugl {

namespace {

class ScopedLocker {
public:
    ScopedLocker(pthread_mutex_t* mutex) : mMutex(mutex) {
        pthread_mutex_lock(mMutex);
    }

    ~ScopedLocker() {
        pthread_mutex_unlock(mMutex);
    }
private:
    pthread_mutex_t* mMutex;
};

}  // namespace

Thread::Thread() :
    mThread((pthread_t)NULL),
    mLock(),
    mJoined(false),
    mExitStatus(0),
    mIsRunning(false) {
    pthread_mutex_init(&mLock, NULL);
}

Thread::~Thread() {
    assert(!mIsRunning);
    assert(mJoined);
    pthread_mutex_destroy(&mLock);
}

bool Thread::start() {
    bool ret = true;
    pthread_mutex_lock(&mLock);
    mIsRunning = true;
    if (pthread_create(&mThread, NULL, thread_main, this)) {
        ret = false;
        mIsRunning = false;
    }
    pthread_mutex_unlock(&mLock);
    return ret;
}

bool Thread::wait(intptr_t *exitStatus) {
    {
        ScopedLocker locker(&mLock);
        if (!mIsRunning) {
            // Thread already stopped.
            if (exitStatus) {
                *exitStatus = mExitStatus;
            }
            if (!mJoined) {
                // reclaim thread stack
                pthread_join(mThread, NULL);
                mJoined = true;
            }
            return true;
        }
    }

    // NOTE: Do not hold the lock when waiting for the thread to ensure
    // it can update mIsRunning and mExitStatus properly in thread_main
    // without blocking.
    void *retval;
    if (pthread_join(mThread, &retval)) {
        return false;
    }
    if (exitStatus) {
        *exitStatus = (intptr_t)retval;
    }
    // Note: Updating mJoined must be performed inside the lock to avoid
    //       race conditions between two threads waiting for the same thread
    //       that just completed its execution.
    {
        ScopedLocker locker(&mLock);
        mJoined = true;
    }
    return true;
}

bool Thread::tryWait(intptr_t *exitStatus) {
    ScopedLocker locker(&mLock);
    if (mIsRunning) {
        return false;
    }
    if (!mJoined) {
        // Reclaim stack.
        pthread_join(mThread, NULL);
        mJoined = true;
    }
    if (exitStatus) {
        *exitStatus = mExitStatus;
    }
    return true;
}

// static
void* Thread::thread_main(void *arg) {
    Thread* self = reinterpret_cast<Thread*>(arg);
    intptr_t ret = self->main();

    pthread_mutex_lock(&self->mLock);
    self->mIsRunning = false;
    self->mExitStatus = ret;
    pthread_mutex_unlock(&self->mLock);

    ::emugl::ThreadStore::OnThreadExit();

    return (void*)ret;
}

}  // namespace emugl
