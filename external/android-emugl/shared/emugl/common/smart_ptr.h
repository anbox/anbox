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
#ifndef EMUGL_SMART_PTR_H
#define EMUGL_SMART_PTR_H

#include <stddef.h>

namespace emugl {

// Hidden atomic ref-counting implementation.
class RefCount;

// Base class for all templated SmartPtr<> instances. Reduces
// template expansion and code. Consider this to be an implementation
// detail of SmartPtr<>, so don't rely on anything here.
class SmartPtrBase {
public:
    // Defrault constructor.
    SmartPtrBase() : mPtr(NULL), mRefCount(NULL) {}

    // Normal constructor. This takes ownership of |ptr|, though only
    // template instances are capable of destroying the object.
    explicit SmartPtrBase(void* ptr);

    // Copy-constructor, this increments the reference count.
    SmartPtrBase(const SmartPtrBase& other);

    // Assignment operator, also increments the reference count.
    SmartPtrBase& operator=(const SmartPtrBase& other);

    // Nothing happens in this destructor, the real work must be performed
    // in subclasses.
    ~SmartPtrBase() {}


    // Used to enable 'if (smart_ptr) { ... }' properly.
    operator void*() const {
        return mPtr;
    }

    // Return internal reference count value, only use for unit testing.
    int getRefCount() const;

protected:
    // Used internally to increment the reference count.
    void addRef();

    // Copy the |other| into this instance, returns the old pointer value
    // if it needs to be destroyed by the caller, or NULL otherwise.
    void* copyFrom(const SmartPtrBase& other);

    // Used internally to decrement the reference count, if it reaches 0,
    // returns the pointer to be destroyed, NULL otherwise.
    void* release();

    void* mPtr;
    RefCount* mRefCount;
};


// The real template class to be used for smart pointers.
// Typical uses:
//
//     SmartPtr<Foo>  ptr(new Foo());   // takes ownership.
//     SmartPtr<Foo>  ptr2;             // empty pointer.
//     ptr2 = ptr;                      // copies pointer + increment reference count.
//     Foo* obj = ptr.Ptr();            // access pointed object.
//     ptr->DoStuff();                  // operate directly on pointed object.
//     (*ptr)->DoStuff();               // same here.
//
// On scope exit, the internal reference count is decremented and the
// object is deleted automatically when it reaches 0, indicating that
// there are no more owners.
//
// IMPORTANT: You need to be sure that only one 'chain' of smart pointers
// own a given object. I.e. the following is incorrect:
//
//     Foo* foo = new Foo();      // create new instance.
//     SmartPtr<Foo>  ptr(foo);   // |ptr| takes ownership of |foo|.
//     SmartPtr<Foo>  ptr2(foo);  // |ptr2| takes also ownership of |foo|.
//
// The problem is that |ptr| and |ptr2| don't know anything about each
// other, and will not share the same reference count. Once a smart pointer
// owns an object, only use other smart pointers that are copy-constructed
// or assigned with the initial one to keep everything consistent.
template <class T>
class SmartPtr : public emugl::SmartPtrBase {
public:
    // Default constructor. The instance holds a NULL pointer.
    SmartPtr() : SmartPtrBase() {}

    // Regular constructor, takes ownership of |ptr|.
    explicit SmartPtr(T* ptr) : SmartPtrBase(ptr) {}

    // Copy-constructor, |this| and |other| will share the same internal
    // reference count, which is incremented by 1.
    SmartPtr(const SmartPtr& other)
            : SmartPtrBase(reinterpret_cast<const SmartPtrBase&>(other)) {}

    // Assignment operator, same semantics as copy-constructor.
    SmartPtr& operator=(const SmartPtr& other) {
      void* old_ptr = copyFrom(static_cast<const SmartPtrBase&>(other));
      if (old_ptr)
        delete reinterpret_cast<T*>(old_ptr);
      return *this;
    }

    // Destructor, decrements reference count and destroys the object
    // if it reaches 0 (indicating this was the last owning smart pointer).
    ~SmartPtr() {
        void* ptr = release();
        if (ptr)
          delete reinterpret_cast<T*>(ptr);
    }

    // Return owned object instance, or NULL.
    T* Ptr() const {
        return reinterpret_cast<T*>(mPtr);
    }

    // Return owned object instance, or NULL
    const T* constPtr() const {
        return reinterpret_cast<const T*>(mPtr);
    }

    // Operate directly on owned object.
    T* operator->() const {
        return Ptr();
    }

    // Return reference to owned object.
    T& operator*() const {
        return *Ptr();
    }
};

}  // namespace emugl

#endif // EMUGL_SMART_PTR_H
