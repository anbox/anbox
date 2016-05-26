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

#ifndef EMUGL_COMMON_POD_VECTOR_H
#define EMUGL_COMMON_POD_VECTOR_H


#include <stddef.h>

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS  1
#endif
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS 1
#endif
#include <stdint.h>

#ifndef SIZE_MAX
#error "You must define __STDC_LIMIT_MACROS before including <stddint.h>"
#endif

namespace emugl {

// A PodVector is a templated vector-like type that is used to store
// POD-struct compatible items only. This allows the implementation to
// use ::memmove() to move items, and also malloc_usable_size() to
// determine the best capacity.
//
// std::vector<> is capable of doing this in theory, using horrible
// templating tricks that make error messages very difficult to
// understand.
//
// Note that a PodVector can be used to store items that contain pointers,
// as long as these do not point to items in the same container.
//
// The PodVector provides methods that also follow the std::vector<>
// conventions, i.e. push_back() is an alias for append().
//

// NOTE: This is a re-implementation of
// external/qemu/android/base/containers/PodVector.h for emugl.

// PodVectorBase is a base, non-templated, implementation class that all
// PodVector instances derive from. This is used to reduce template
// specialization. Do not use directly, i..e it's an implementation detail.
class PodVectorBase {
protected:
    PodVectorBase() : mBegin(NULL), mEnd(NULL), mLimit(NULL) {}
    explicit PodVectorBase(const PodVectorBase& other);
    PodVectorBase& operator=(const PodVectorBase& other);
    ~PodVectorBase();

    bool empty() const { return mEnd == mBegin; }

    size_t byteSize() const { return mEnd - mBegin; }

    size_t byteCapacity() const { return mLimit - mBegin; }

    void* begin() { return mBegin; }
    const void* begin() const { return mBegin; }
    void* end() { return mEnd; }
    const void* end() const { return mEnd; }

    void* itemAt(size_t pos, size_t itemSize) {
        return mBegin + pos * itemSize;
    }

    const void* itemAt(size_t pos, size_t itemSize) const {
        return mBegin + pos * itemSize;
    }

    void assignFrom(const PodVectorBase& other);

    inline size_t itemCount(size_t itemSize) const {
        return byteSize() / itemSize;
    }

    inline size_t itemCapacity(size_t itemSize) const {
        return byteCapacity() / itemSize;
    }

    inline size_t maxItemCapacity(size_t itemSize) const {
        return SIZE_MAX / itemSize;
    }

    void resize(size_t newSize, size_t itemSize);
    void reserve(size_t newSize, size_t itemSize);

    void removeAt(size_t index, size_t itemSize);
    void* insertAt(size_t index, size_t itemSize);
    void swapAll(PodVectorBase* other);

    char* mBegin;
    char* mEnd;
    char* mLimit;

private:
    void initFrom(const void* from, size_t fromLen);
};


// A PodVector<T> holds a vector (dynamically resizable array) or items
// that must be POD-struct compatible (i.e. they cannot have constructors,
// destructors, or virtual members). This allows the implementation to be
// small, fast and efficient, memory-wise.
//
// If you want to implement a vector of C++ objects, consider using
// std::vector<> instead, but keep in mind that this implies a non-trivial
// cost when appending, inserting, removing items in the collection.
//
template <typename T>
class PodVector : public PodVectorBase {
public:
    // Default constructor for an empty PodVector<T>
    PodVector() : PodVectorBase() {}

    // Copy constructor. This copies all items from |other| into
    // the new instance with ::memmove().
    PodVector(const PodVector& other) : PodVectorBase(other) {}

    // Assignment operator.
    PodVector& operator=(const PodVector& other) {
        this->assignFrom(other);
        return *this;
    }

    // Destructor, this simply releases the internal storage block that
    // holds all the items, but doesn't touch them otherwise.
    ~PodVector() {}

    // Return true iff the PodVector<T> instance is empty, i.e. does not
    // have any items.
    bool empty() const { return PodVectorBase::empty(); }

    // Return the number of items in the current PodVector<T> instance.
    size_t size() const { return PodVectorBase::itemCount(sizeof(T)); }

    // Return the current capacity in the current PodVector<T> instance.
    // Do not use directly, except if you know what you're doing. Try to
    // use resize() or reserve() instead.
    size_t capacity() const {
        return PodVectorBase::itemCapacity(sizeof(T));
    }

    // Return the maximum capacity of any PodVector<T> instance.
    static inline size_t maxCapacity() { return SIZE_MAX / sizeof(T); }

    // Resize the vector to ensure it can hold |newSize|
    // items. This may or may not call reserve() under the hood.
    // It's a fatal error to try to resize above maxCapacity().
    void resize(size_t newSize) {
        PodVectorBase::resize(newSize, sizeof(T));
    }

    // Resize the vector's storage array to ensure that it can hold at
    // least |newSize| items. It's a fatal error to try to resize above
    // maxCapacity().
    void reserve(size_t newSize) {
        PodVectorBase::reserve(newSize, sizeof(T));
    }

    // Return a pointer to the first item in the vector. This is only
    // valid until the next call to any function that changes the size
    // or capacity of the vector. Can be NULL if the vector is empty.
    T* begin() {
        return reinterpret_cast<T*>(PodVectorBase::begin());
    }

    // Return a constant pointer to the first item in the vector. This is
    // only valid until the next call to any function that changes the
    // size of capacity of the vector.
    const T* begin() const {
        return reinterpret_cast<T*>(PodVectorBase::begin());
    }

    // Return a pointer past the last item in the vector. I.e. if the
    // result is not NULL, then |result - 1| points to the last item.
    // Can be NULL if the vector is empty.
    T* end() {
        return reinterpret_cast<T*>(PodVectorBase::end());
    }

    // Return a constant pointer past the last item in the vector. I.e. if
    // the result is not NULL, then |result - 1| points to the last item.
    // Can be NULL if the vector is empty.
    const T* end() const {
        return reinterpret_cast<T*>(PodVectorBase::end());
    }

    // Returns a reference to the item a position |index| in the vector.
    // It may be a fatal error to access an out-of-bounds position.
    T& operator[](size_t index) {
        return *reinterpret_cast<T*>(
                PodVectorBase::itemAt(index, sizeof(T)));
    }

    const T& operator[](size_t index) const {
        return *reinterpret_cast<const T*>(
                PodVectorBase::itemAt(index, sizeof(T)));
    }

    // Increase the vector's size by 1, then move all items past a given
    // position to the right. Return the position of the insertion point
    // to let the caller copy the content it desires there. It's preferrable
    // to use insert() directly, which will do the item copy for you.
    T* emplace(size_t index) {
        return reinterpret_cast<T*>(
                PodVectorBase::insertAt(index, sizeof(T)));
    }

    // Insert an item at a given position. |index| is the insertion position
    // which must be between 0 and size() included, or a fatal error may
    // occur. |item| is a reference to an item to copy into the array,
    // byte-by-byte.
    void insert(size_t index, const T& item) {
        *(this->emplace(index)) = item;
    }

    // Prepend an item at the start of a vector. This moves all vector items
    // to the right, and is thus costly. |item| is a reference to an item
    // to copy to the start of the vector.
    void prepend(const T& item) {
        *(this->emplace(0U)) = item;
    }

    // Append an item at the end of a vector. Specialized version of insert()
    // which always uses size() as the insertion position.
    void append(const T& item) {
        *(this->emplace(this->size())) = item;
    }

    // Remove the item at a given position. |index| is the position of the
    // item to delete. It must be between 0 and size(), included, or
    // a fatal error may occur. Deleting the item at position size()
    // doesn't do anything.
    void remove(size_t index) {
        PodVectorBase::removeAt(index, sizeof(T));
    }

    void swap(PodVector* other) {
        PodVectorBase::swapAll(other);
    }

    // Compatibility methods for std::vector<>
    void push_back(const T& item) { append(item); }
    void pop() { remove(0U); }

    typedef T* iterator;
    typedef const T* const_iterator;
};

}  // namespace emugl

#endif  // EMUGL_COMMON_POD_VECTOR_H
