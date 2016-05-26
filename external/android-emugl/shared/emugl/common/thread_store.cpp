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

#ifdef _WIN32
#include "emugl/common/lazy_instance.h"
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Set to 1 to print debug messages.
#define DEBUG_THREAD_STORE  0

#if DEBUG_THREAD_STORE
#  define D(...)   do { printf("%s:%d: ", __FUNCTION__, __LINE__); printf(__VA_ARGS__); fflush(stdout); } while (0)
#else
#  define D(...)   ((void)0)
#endif

namespace emugl {

#ifdef _WIN32

namespace {

// The ThreadStore implementation on Windows is very tricky, because
// TlsAlloc() doesn't allow one to provide a destructor function. As
// such threads are expected to destroy all TLS values explicitely.
//
// To solve this issue, this source file provides a static method called
// ThreadStore::OnThreadExit() that must be called when a thread exits,
// which will cleanup all values for the current thread.
//
// But this forces us to track thread-specific values ourselves.

// Maximum amount of thread-specific slots supported by this implementation.
enum {
    kMaxTlsSlots = 64
};

// TlsSlotArray is a thread-specific array of values. Instances will
// be stored in a Win32 TLS value controlled by a single master TLS
// key.
//
typedef void* TlsSlotArray[kMaxTlsSlots];

// Global state shared by all threads
class GlobalState {
public:
    GlobalState() {
        D("Entering\n");
        mMasterTls = TlsAlloc();
        D("Master TLS = %d\n", (int)mMasterTls);
        InitializeCriticalSection(&mSection);
        mLastIndex = 0;
        ::memset(mDestructors, 0, sizeof(mDestructors));
        D("Exiting\n");
    }

    // Register a new TLS key, or return -1 on error (too many keys).
    // |destroy| is the destructor function for the key.
    int registerKey(ThreadStore::Destructor* destroy) {
        D("Entering destroy=%p\n", destroy);
        int ret = -1;
        EnterCriticalSection(&mSection);
        if (mLastIndex < kMaxTlsSlots) {
            ret = mLastIndex++;
            mDestructors[ret] = destroy;
        }
        LeaveCriticalSection(&mSection);
        D("Exiting newKey=%d\n", ret);
        return ret;
    }

    void unregisterKey(int key) {
        D("key=%d\n", key);
        if (key < 0 || key >= kMaxTlsSlots) {
            D("Invalid key\n");
            return;
        }

        // Note: keys are not reusable, but remove the destructor to avoid
        // crashes in leaveCurrentThread() when it points to a function that
        // is going to be unloaded from the process' address space.
        EnterCriticalSection(&mSection);
        mDestructors[key] = NULL;
        LeaveCriticalSection(&mSection);
        D("Exiting\n");
    }

    // Get the current thread-local value for a given |key|.
    void* getValue(int key) const {
        D("Entering key=%d\n", key);
        if (key < 0 || key >= kMaxTlsSlots) {
            D("Invalid key, result=NULL\n");
            return NULL;
        }

        TlsSlotArray* array = getArray();
        void* ret = (*array)[key];
        D("Exiting keyValue=%p\n", ret);
        return ret;
    }

    // Set the current thread-local |value| for a given |key|.
    void setValue(int key, void* value) {
        D("Entering key=%d\n",key);
        if (key < 0 || key >= kMaxTlsSlots) {
            D("Invalid key, returning\n");
            return;
        }

        TlsSlotArray* array = getArray();
        (*array)[key] = value;
        D("Exiting\n");
    }

    // Call this when a thread exits to destroy all its thread-local values.
    void leaveCurrentThread() {
        D("Entering\n");
        TlsSlotArray* array =
                reinterpret_cast<TlsSlotArray*>(TlsGetValue(mMasterTls));
        if (!array) {
            D("Exiting, no thread-local data in this thread\n");
            return;
        }

        for (size_t n = 0; n < kMaxTlsSlots; ++n) {
            void* value = array[n];
            if (value) {
                (*array)[n] = NULL;
                // NOTE: In theory, a destructor could reset the slot to
                // a new value, and we would have to loop in this function
                // in interesting ways. In practice, ignore the issue.
                EnterCriticalSection(&mSection);
                ThreadStore::Destructor* destroy = mDestructors[n];
                LeaveCriticalSection(&mSection);
                if (destroy) {
                    D("Calling destructor %p for key=%d, with value=%p\n",
                      destroy, (int)n, value);
                    (*destroy)(value);
                }
            }
        }
        TlsSetValue(mMasterTls, NULL);
        ::free(array);
        D("Exiting\n");
    }

private:
    // Return the thread-local array of TLS slots for the current thread.
    // Cannot return NULL.
    TlsSlotArray* getArray() const {
        D("Entering\n");
        TlsSlotArray* array =
                reinterpret_cast<TlsSlotArray*>(TlsGetValue(mMasterTls));
        if (!array) {
            array = reinterpret_cast<TlsSlotArray*>(
                    ::calloc(sizeof(*array), 1));
            TlsSetValue(mMasterTls, array);
            D("Allocated new array at %p\n", array);
        } else {
            D("Retrieved array at %p\n", array);
        }
        return array;
    }

    DWORD mMasterTls;
    CRITICAL_SECTION mSection;
    int mLastIndex;
    ThreadStore::Destructor* mDestructors[kMaxTlsSlots];
};

LazyInstance<GlobalState> gGlobalState = LAZY_INSTANCE_INIT;

}  // namespace

ThreadStore::ThreadStore(Destructor* destroy) {
    D("Entering this=%p destroy=%p\n", this, destroy);
    mKey = gGlobalState->registerKey(destroy);
    D("Exiting this=%p key=%d\n", this, mKey);
}

ThreadStore::~ThreadStore() {
    D("Entering this=%p\n", this);
    GlobalState* state = gGlobalState.ptr();
    state->unregisterKey(mKey);
    D("Exiting this=%p\n", this);
}

void* ThreadStore::get() const {
    D("Entering this=%p\n", this);
    void* ret = gGlobalState->getValue(mKey);
    D("Exiting this=%p value=%p\n", this, ret);
    return ret;
}

void ThreadStore::set(void* value) {
    D("Entering this=%p value=%p\n", this, value);
    gGlobalState->setValue(mKey, value);
    D("Exiting this=%p\n", this);
}

// static
void ThreadStore::OnThreadExit() {
    gGlobalState->leaveCurrentThread();
}

#else  // !_WIN32

ThreadStore::ThreadStore(Destructor* destroy) {
    int ret = pthread_key_create(&mKey, destroy);
    if (ret != 0) {
        fprintf(stderr,
                "Could not create thread store key: %s\n",
                strerror(ret));
        exit(1);
    }
}

ThreadStore::~ThreadStore() {
    pthread_key_delete(mKey);
}

#endif  // !_WIN32

}  // namespace emugl
