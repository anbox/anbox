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

namespace emugl {

namespace {

class ScopedLocker {
public:
    ScopedLocker(CRITICAL_SECTION* section) : mSection(section) {
        EnterCriticalSection(mSection);
    }

    ~ScopedLocker() {
        LeaveCriticalSection(mSection);
    }
private:
    CRITICAL_SECTION* mSection;
};

}  // namespace

Thread::Thread() :
    mThread(INVALID_HANDLE_VALUE),
    mThreadId(0),
    mExitStatus(0),
    mIsRunning(false) {
    InitializeCriticalSection(&mLock);
}

Thread::~Thread() {
    if(mThread != INVALID_HANDLE_VALUE) {
        CloseHandle(mThread);
    }
    DeleteCriticalSection(&mLock);
}

bool Thread::start() {
    ScopedLocker locker(&mLock);

    bool ret = true;
    mIsRunning = true;
    mThread = CreateThread(NULL, 0, &Thread::thread_main, this, 0, &mThreadId);
    if (!mThread) {
        ret = false;
        mIsRunning = false;
    }
    return ret;
}

bool Thread::wait(intptr_t* exitStatus) {
    {
        ScopedLocker locker(&mLock);
        if (!mIsRunning) {
            // Thread already stopped.
            if (exitStatus) {
                *exitStatus = mExitStatus;
            }
            return true;
        }
    }

    // NOTE: Do not hold lock during wait to aloow thread_main to
    // properly update mIsRunning and mExitStatus on thread exit.
    if (WaitForSingleObject(mThread, INFINITE) == WAIT_FAILED) {
        return false;
    }

    if (exitStatus) {
        ScopedLocker locker(&mLock);
        *exitStatus = mExitStatus;
    }
    return true;
}

bool Thread::tryWait(intptr_t* exitStatus) {
    ScopedLocker locker(&mLock);

    if (mIsRunning && WaitForSingleObject(mThread, 0) != WAIT_OBJECT_0) {
        return false;
    }

    if (exitStatus) {
        *exitStatus = mExitStatus;
    }
    return true;
}

// static
DWORD WINAPI Thread::thread_main(void *arg)
{
    Thread* self = reinterpret_cast<Thread*>(arg);
    intptr_t ret = self->main();

    EnterCriticalSection(&self->mLock);
    self->mIsRunning = false;
    self->mExitStatus = ret;
    LeaveCriticalSection(&self->mLock);

    // Ensure all thread-local values are released for this thread.
    ::emugl::ThreadStore::OnThreadExit();

    return static_cast<DWORD>(ret);
}

}  // namespace emugl
