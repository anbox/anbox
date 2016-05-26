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

#include "emugl/common/mutex.h"

#include <gtest/gtest.h>

namespace emugl {

namespace {

// A simple thread instance that does nothing at all and exits immediately.
class EmptyThread : public ::emugl::Thread {
public:
    intptr_t main() { return 42; }
};

class CountingThread : public ::emugl::Thread {
public:
    class State {
    public:
        State() : mLock(), mCount(0) {}
        ~State() {}

        void increment() {
            mLock.lock();
            mCount++;
            mLock.unlock();
        }

        int count() const {
            int ret;
            mLock.lock();
            ret = mCount;
            mLock.unlock();
            return ret;
        }

    private:
        mutable Mutex mLock;
        int mCount;
    };

    CountingThread(State* state) : mState(state) {}

    intptr_t main() {
        mState->increment();
        return 0;
    }

private:
    State* mState;
};

class WaitingThread : public ::emugl::Thread {
public:
    WaitingThread(Mutex* lock) : mLock(lock) {}

    intptr_t main() {
        // Try to acquire lock.
        mLock->lock();

        // Then try to release it.
        mLock->unlock();
        return 0;
    }
private:
    Mutex* mLock;
};

}  // namespace

TEST(ThreadTest, WaitForSimpleThread) {
    Thread* thread = new EmptyThread();
    EXPECT_TRUE(thread);
    EXPECT_TRUE(thread->start());
    intptr_t status;
    EXPECT_TRUE(thread->wait(&status));
    EXPECT_EQ(42, status);
}

TEST(ThreadTest, WaitForMultipleThreads) {
    CountingThread::State state;
    const size_t kMaxThreads = 100;
    Thread* threads[kMaxThreads];

    // Create all threads.
    for (size_t n = 0; n < kMaxThreads; ++n) {
        threads[n] = new CountingThread(&state);
        EXPECT_TRUE(threads[n]) << "thread " << n;
    }

    // Start them all.
    for (size_t n = 0; n < kMaxThreads; ++n) {
        EXPECT_TRUE(threads[n]->start()) << "thread " << n;
    }

    // Wait for them all.
    for (size_t n = 0; n < kMaxThreads; ++n) {
        EXPECT_TRUE(threads[n]->wait(NULL)) << "thread " << n;
    }

    // Check state.
    EXPECT_EQ((int)kMaxThreads, state.count());

    // Delete them all.
    for (size_t n = 0; n < kMaxThreads; ++n) {
        delete threads[n];
    }
}

TEST(ThreadTest, TryWaitForMultipleThreads) {
    Mutex lock;
    const size_t kMaxThreads = 100;
    Thread* threads[kMaxThreads];

    // Create all threads.
    for (size_t n = 0; n < kMaxThreads; ++n) {
        threads[n] = new WaitingThread(&lock);
        EXPECT_TRUE(threads[n]) << "thread " << n;
    }

    // Acquire the lock, this will block all threads.
    lock.lock();

    // Start them all.
    for (size_t n = 0; n < kMaxThreads; ++n) {
        EXPECT_TRUE(threads[n]->start()) << "thread " << n;
    }

    // Check that tryWait() fails for all threads.
    for (size_t n = 0; n < kMaxThreads; ++n) {
        EXPECT_FALSE(threads[n]->tryWait(NULL)) << "thread" << n;
    }

    // Release the lock, this will unblock all threads.
    lock.unlock();

    // Wait for them all.
    for (size_t n = 0; n < kMaxThreads; ++n) {
        EXPECT_TRUE(threads[n]->wait(NULL)) << "thread " << n;
        EXPECT_TRUE(threads[n]->tryWait(NULL)) << "thread " << n;
    }

    // Delete them all.
    for (size_t n = 0; n < kMaxThreads; ++n) {
        delete threads[n];
    }
}

}  // namespace emugl
