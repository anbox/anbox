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

#ifndef EMUGL_MUTEX_H
#define EMUGL_MUTEX_H

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN 1
#  include <windows.h>
#else
#  include <pthread.h>
#endif

namespace emugl {

class ConditionVariable;

// Simple wrapper class for mutexes.
class Mutex {
public:
    // Constructor.
    Mutex() {
#ifdef _WIN32
        ::InitializeCriticalSection(&mLock);
#else
        ::pthread_mutex_init(&mLock, NULL);
#endif
    }

    // Destructor.
    ~Mutex() {
#ifdef _WIN32
        ::DeleteCriticalSection(&mLock);
#else
        ::pthread_mutex_destroy(&mLock);
#endif
    }

    // Acquire the mutex.
    void lock() {
#ifdef _WIN32
      ::EnterCriticalSection(&mLock);
#else
      ::pthread_mutex_lock(&mLock);
#endif
    }

    // Release the mutex.
    void unlock() {
#ifdef _WIN32
       ::LeaveCriticalSection(&mLock);
#else
       ::pthread_mutex_unlock(&mLock);
#endif
    }

    // Helper class to lock / unlock a mutex automatically on scope
    // entry and exit.
    class AutoLock {
    public:
        AutoLock(Mutex& mutex) : mMutex(&mutex) {
            mMutex->lock();
        }

        ~AutoLock() {
            mMutex->unlock();
        }
    private:
        Mutex* mMutex;
    };

private:
#ifdef _WIN32
    CRITICAL_SECTION mLock;
#else
    friend class ConditionVariable;
    pthread_mutex_t mLock;
#endif

};

}  // namespace emugl

#endif  // EMUGL_MUTEX_H
