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

#ifndef EMUGL_COMMON_THREAD_STORE_H
#define EMUGL_COMMON_THREAD_STORE_H

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN 1
#  include <windows.h>
#else
#  include <pthread.h>
#endif

namespace emugl {

// A class to model storage of thread-specific values, that can be
// destroyed on thread exit.
//
// Note that on Windows, a thread must call OnThreadExit() explicitly
// here to ensure that the values are probably discarded. This is an
// unfortunate requirement of the Win32 API, which doesn't support
// destructors at all.
//
// There are various hacks on the web to try to achieve this automatically
// (e.g. [1]) but they rely on using the Microsoft build tools,
// which doesn't work for us.
//
// Note another important issue with ThreadStore instances: if you create
// one instance in a shared library, you need to make sure that it is
// always destroyed before the library is unloaded. Otherwise, future
// thread exit will likely crash, due to calling a destructor function
// that is no longer in the process' address space.
//
// Finally, destroying an instance does _not_ free the corresponding values,
// because doing so properly requires coordinating all participating threads,
// which is impossible to achieve in the most general case. Thus, consider
// that thread-local values are always leaked on library unload, or on
// program exit.
//
// [1] http://stackoverflow.com/questions/14538159/about-tls-callback-in-windows

class ThreadStore {
public:
    // Type of a function used to destroy a thread-specific value that
    // was previously assigned by calling set().
    typedef void (Destructor)(void* value);

    // Initialize instance so that is hold keys that must be destroyed
    // on thread exit by calling |destroy|.
    explicit ThreadStore(Destructor* destroy);

    // NOTE: Destructor don't free the thread-local values, but are required
    // to avoid crashes (see note above).
    ~ThreadStore();

    // Retrieve current thread-specific value from store.
#ifdef _WIN32
    void* get() const;
#else
    inline void* get() const {
        return pthread_getspecific(mKey);
    }
#endif

    // Set the new thread-specific value.
#ifdef _WIN32
    void set(void* value);
#else
    inline void set(void* value) {
        pthread_setspecific(mKey, value);
    }
#endif

#ifdef _WIN32
    // Each thread should call this function on exit to ensure that
    // all corresponding TLS values are properly freed.
    static void OnThreadExit();
#else
    // Nothing to do on Posix.
    static inline void OnThreadExit() {}
#endif

private:
    // Ensure you can't create an empty ThreadStore instance, or simply
    // copy it in any way.
    ThreadStore();
    ThreadStore(const ThreadStore&);
    ThreadStore& operator=(const ThreadStore&);

#ifdef _WIN32
    int mKey;
#else
    pthread_key_t mKey;
#endif
};

}  // namespace emugl

#endif  // EMUGL_COMMON_THREAD_STORE_H
