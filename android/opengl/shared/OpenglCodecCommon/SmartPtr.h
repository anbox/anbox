/*
* Copyright (C) 2011 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef __SMART_PTR_H
#define __SMART_PTR_H

#include <cutils/threads.h>
#include <cutils/atomic.h>

template <class T, bool threadSafe = false>
class SmartPtr
{
public:
    explicit SmartPtr(T* ptr = (T*)NULL) {
        if (threadSafe) {
            m_lock = new mutex_t;
            mutex_init(m_lock);
        }
        else m_lock = NULL;

        m_ptr = ptr;
        if (ptr)
           m_pRefCount = new int32_t(1);
        else
           m_pRefCount = NULL;
    }

    SmartPtr<T,threadSafe>(const SmartPtr<T,false>& rhs) {
        if (threadSafe) {
            m_lock = new mutex_t;
            mutex_init(m_lock);
        }
        else m_lock = NULL;

        m_pRefCount = rhs.m_pRefCount;
        m_ptr       = rhs.m_ptr;
        use();
    }

    SmartPtr<T,threadSafe>(SmartPtr<T,true>& rhs) {
        if (threadSafe) {
            m_lock = new mutex_t;
            mutex_init(m_lock);
        }
        else m_lock = NULL;

        if (rhs.m_lock) mutex_lock(rhs.m_lock);
        m_pRefCount = rhs.m_pRefCount;
        m_ptr       = rhs.m_ptr;
        use();
        if (rhs.m_lock) mutex_unlock(rhs.m_lock);
    }

    ~SmartPtr() {
        if (m_lock) mutex_lock(m_lock);
        release();
        if (m_lock)
        {
            mutex_unlock(m_lock);
            mutex_destroy(m_lock);
            delete m_lock;
        }
    }

    T* Ptr() const {
        return m_ptr;
    }

    const T* constPtr() const
    {
        return m_ptr;
    }

    T* operator->() const {
        return m_ptr;
    }

    T& operator*() const {
        return *m_ptr;
    }

    operator void*() const {
        return (void *)m_ptr;
    }

    // This gives STL lists something to compare.
    bool operator <(const SmartPtr<T>& t1) const {
        return m_ptr < t1.m_ptr;
    }

    SmartPtr<T,threadSafe>& operator=(const SmartPtr<T,false>& rhs)
    {
        if (m_ptr == rhs.m_ptr)
            return *this;

        if (m_lock) mutex_lock(m_lock);
        release();
        m_pRefCount = rhs.m_pRefCount;
        m_ptr       = rhs.m_ptr;
        use();
        if (m_lock) mutex_unlock(m_lock);

        return *this;
    }

    SmartPtr<T,threadSafe>& operator=(SmartPtr<T,true>& rhs)
    {
        if (m_ptr == rhs.m_ptr)
            return *this;

        if (m_lock) mutex_lock(m_lock);
        release();
        if (rhs.m_lock) mutex_lock(rhs.m_lock);
        m_pRefCount = rhs.m_pRefCount;
        m_ptr       = rhs.m_ptr;
        use();
        if (rhs.m_lock) mutex_unlock(rhs.m_lock);
        if (m_lock) mutex_unlock(m_lock);

        return *this;
    }

private:
    int32_t  *m_pRefCount;
    mutex_t  *m_lock;
    T* m_ptr;

    // Increment the reference count on this pointer by 1.
    int use() {
        if (!m_pRefCount) return 0;
        return android_atomic_inc(m_pRefCount) + 1;
    }

    // Decrement the reference count on the pointer by 1.
    // If the reference count goes to (or below) 0, the pointer is deleted.
    int release() {
        if (!m_pRefCount) return 0;

        int iVal = android_atomic_dec(m_pRefCount);
        if (iVal > 1)
            return iVal - 1;

        delete m_pRefCount;
        m_pRefCount = NULL;

        if (m_ptr) {
            delete m_ptr;
            m_ptr = NULL;
        }
        return 0;
    }

};

#endif // of  __SMART_PTR_H
