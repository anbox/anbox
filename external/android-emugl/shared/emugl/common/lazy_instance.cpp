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

#include "emugl/common/lazy_instance.h"

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN 1
#  include <windows.h>
#else
#  include <sched.h>
#endif

namespace emugl {
namespace internal {

typedef LazyInstanceState::AtomicType AtomicType;

#if defined(__GNUC__)
static inline void compilerBarrier() {
    __asm__ __volatile__ ("" : : : "memory");
}
#else
#error "Your compiler is not supported"
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) || defined(__aarch64__)
#  define acquireBarrier() compilerBarrier()
#  define releaseBarrier() compilerBarrier()
#else
#  error "Your CPU is not supported"
#endif

static inline AtomicType loadAcquire(AtomicType volatile* ptr) {
    AtomicType ret = *ptr;
    acquireBarrier();
    return ret;
}

static inline void storeRelease(AtomicType volatile* ptr, AtomicType value) {
    releaseBarrier();
    *ptr = value;
}

static int atomicCompareAndSwap(AtomicType volatile* ptr,
                                int expected,
                                int value) {
#ifdef _WIN32
    return InterlockedCompareExchange(ptr, value, expected);
#elif defined(__GNUC__)
    return __sync_val_compare_and_swap(ptr, expected, value);
#else
#error "Your compiler is not supported"
#endif
}

static void yieldThread() {
#ifdef _WIN32
    ::Sleep(0);
#else
    sched_yield();
#endif
}

bool LazyInstanceState::inInitState() {
    return loadAcquire(&mState) == STATE_INIT;
}

bool LazyInstanceState::needConstruction() {
    AtomicType state = loadAcquire(&mState);
    if (mState == STATE_DONE)
        return false;

    state = atomicCompareAndSwap(&mState, STATE_INIT, STATE_CONSTRUCTING);
    if (state == STATE_INIT)
        return true;

    do {
        yieldThread();
        state = loadAcquire(&mState);
    } while (state != STATE_DONE);

    return false;
}

void LazyInstanceState::doneConstructing() {
    storeRelease(&mState, STATE_DONE);
}

}  // namespace internal
}  // namespace emugl
