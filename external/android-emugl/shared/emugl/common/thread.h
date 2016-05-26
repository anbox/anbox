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

#ifndef EMUGL_COMMON_THREAD_H
#define EMUGL_COMMON_THREAD_H

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <stdint.h>

namespace emugl {

// Wrapper class for platform-specific threads.
// To create your own thread, define a sub-class of emugl::Thread
// and override its main() method.
//
// For example:
//
//    class MyThread : public emugl::Thread {
//    public:
//        MyThread() : Thread() {}
//
//        virtual intptr_t main() {
//            ... main thread loop implementation
//            return 0;
//        }
//    };
//
//    ...
//
//    // Create new instance, but does not start it.
//    MyThread* thread = new MyThread();
//
//    // Start the thread.
//    thread->start();
//
//    // Wait for thread completion, and gets result into |exitStatus|.
//    int exitStatus;
//    thread->wait(&exitStatus);
//
class Thread {
public:
    // Public constructor.
    Thread();

    // Virtual destructor.
    virtual ~Thread();

    // Override this method in your own thread sub-classes. This will
    // be called when start() is invoked on the Thread instance.
    virtual intptr_t main() = 0;

    // Start a thread instance. Return true on success, false otherwise
    // (e.g. if the thread was already started or terminated).
    bool start();

    // Wait for thread termination and retrieve exist status into
    // |*exitStatus|. Return true on success, false otherwise.
    // NOTE: |exitStatus| can be NULL.
    bool  wait(intptr_t *exitStatus);

    // Check whether a thread has terminated. On success, return true
    // and sets |*exitStatus|. On failure, return false.
    // NOTE: |exitStatus| can be NULL.
    bool tryWait(intptr_t *exitStatus);

private:
#ifdef _WIN32
    static DWORD WINAPI thread_main(void* arg);

    HANDLE mThread;
    DWORD mThreadId;
    CRITICAL_SECTION mLock;
#else // !WIN32
    static void* thread_main(void* arg);

    pthread_t mThread;
    pthread_mutex_t mLock;
    bool mJoined;
#endif
    intptr_t mExitStatus;
    bool mIsRunning;
};

}  // namespace emugl

#endif  // EMUGL_COMMON_THREAD_H

