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

#include "emugl/common/thread_store.h"

#include "emugl/common/mutex.h"
#include "emugl/common/testing/test_thread.h"

#include <gtest/gtest.h>

namespace emugl {

namespace {

// Helper class used to count instance creation and destruction.
class StaticCounter {
public:
    enum {
        kMaxInstances = 1000,
    };

    StaticCounter() {
        Mutex::AutoLock lock(mMutex);
        if (mCreationCount < kMaxInstances)
            mInstances[mCreationCount] = this;
        mCreationCount++;
    }

    ~StaticCounter() {
        Mutex::AutoLock lock(mMutex);
        mDestructionCount++;
    }

    static void reset() {
        Mutex::AutoLock lock(mMutex);
        mCreationCount = 0;
        mDestructionCount = 0;
    }

    static size_t getCreationCount() {
        Mutex::AutoLock lock(mMutex);
        return mCreationCount;
    }

    static size_t getDestructionCount() {
        Mutex::AutoLock lock(mMutex);
        return mDestructionCount;
    }

    static void freeAll() {
        for (size_t n = 0; n < kMaxInstances; ++n)
            delete mInstances[n];
    }

private:
    static Mutex mMutex;
    static size_t mCreationCount;
    static size_t mDestructionCount;
    static StaticCounter* mInstances[kMaxInstances];
};

Mutex StaticCounter::mMutex;
size_t StaticCounter::mCreationCount = 0;
size_t StaticCounter::mDestructionCount = 0;
StaticCounter* StaticCounter::mInstances[kMaxInstances];

}  // namespace

// Just check that we can create a new ThreadStore with an empty
// destructor, and use it in the current thread.
TEST(ThreadStore, MainThreadWithoutDestructor) {
    ThreadStore store(NULL);
    static int x = 42;
    store.set(&x);
    EXPECT_EQ(&x, store.get());
}

// The following test checks that exiting a thread correctly deletes
// any thread-local value stored in it.
static void simplyDestroy(void* value) {
    delete (StaticCounter*) value;
}

static void* simpleThreadFunc(void* param) {
    ThreadStore* store = static_cast<ThreadStore*>(param);
    store->set(new StaticCounter());
    ThreadStore::OnThreadExit();
    return NULL;
}

TEST(ThreadStore, ThreadsWithDestructor) {
    ThreadStore store(simplyDestroy);
    const size_t kNumThreads = 1000;
    TestThread* threads[kNumThreads];
    StaticCounter::reset();

    for (size_t n = 0; n < kNumThreads; ++n) {
        threads[n] = new TestThread(&simpleThreadFunc, &store);
    }
    for (size_t n = 0; n < kNumThreads; ++n) {
        threads[n]->join();
    }

    EXPECT_EQ(kNumThreads, StaticCounter::getCreationCount());
    EXPECT_EQ(kNumThreads, StaticCounter::getDestructionCount());

    for (size_t n = 0; n < kNumThreads; ++n) {
        delete threads[n];
    }
}

TEST(ThreadStore, ThreadsWithoutDestructor) {
    ThreadStore store(NULL);
    const size_t kNumThreads = 1000;
    TestThread* threads[kNumThreads];
    StaticCounter::reset();

    for (size_t n = 0; n < kNumThreads; ++n) {
        threads[n] = new TestThread(&simpleThreadFunc, &store);
    }
    for (size_t n = 0; n < kNumThreads; ++n) {
        threads[n]->join();
    }

    EXPECT_EQ(kNumThreads, StaticCounter::getCreationCount());
    EXPECT_EQ(0U, StaticCounter::getDestructionCount());

    StaticCounter::freeAll();

    for (size_t n = 0; n < kNumThreads; ++n) {
        delete threads[n];
    }
}

}  // namespace emugl
