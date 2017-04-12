// Copyright 2016 The Android Open Source Project
//
// This software is licensed under the terms of the GNU General Public
// License version 2, as published by the Free Software Foundation, and
// may be copied, distributed, and modified under those terms.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#pragma once

#include "anbox/common/type_traits.h"

#include <algorithm>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>

#include <stddef.h>
#include <stdlib.h>

//
// SmallVector<T>, SmallFixedVector<T, SmallSize>
//
// This header defines a replacement for a std::vector<> that uses small buffer
// optimization technique - for some preset number of elements |SmallSize| it
// stores them inside of the object, and falls back to the dynamically allocated
// array only if one needs to add more elements.
// This is useful for the performance-critical places where common number of
// processed items is small, but it may still be quite large for a stack array.
//
// SmallFixedVector<> is the class you use to store elements, while
// SmallVector<> is its base class that erases the small size from the type.
//
// NOTE: SmallVector<> cannot guarantee std::vector<>'s iterator invalidation
//   rules for move() and swap() operations - std::vector<>s just exchange
//   their iterators on swap() and pass the moved ones over, while SmallVector<>
//   may leave the iterators pointing to nowhere if they were for the in-place
//   array storage.
//
// Currenly only a limited subset of std::vector<>'s operations is implemented,
// but fill free to add the ones you need.
//

namespace anbox {
namespace common {

//
// Forward-declare the 'real' small vector class.
template <class T, size_t S>
class SmallFixedVector;

//
// SmallVector<T> - an interface for a small-buffer-optimized vector.
// It hides the fixed size from its type, so one can use it to pass small
// vectors around and not leak the buffer size to all callers:
//
//  void process(SmallVector<Foo>& data);
//  ...
//  ...
//  SmallFixedVector<Foo, 100> aLittleBitOfFoos = ...;
//  process(aLittleBitOfFoos);
//  ...
//  SmallFixedVector<Foo, 1000> moreFoos = ...;
//  process(moreFoos);
//
template <class T>
class SmallVector {
  // Make them friends so SmallFixedVector is able to refer to SmallVector's
  // protected members in static_assert()s.
  template <class U, size_t S>
  friend class SmallFixedVector;

 public:
  // Common set of type aliases.
  using value_type = T;
  using iterator = T*;
  using const_iterator = const T*;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using size_type = size_t;

  // It's ok to delete SmallVector<> through the base class - dtor() actually
  // takes care of all living elements and the allocated memory.
  ~SmallVector() { dtor(); }

  // std::vector<> interface operations.
  iterator begin() { return mBegin; }
  const_iterator begin() const { return mBegin; }
  const_iterator cbegin() const { return mBegin; }

  iterator end() { return mEnd; }
  const_iterator end() const { return mEnd; }
  const_iterator cend() const { return mEnd; }

  size_type size() const { return end() - begin(); }
  size_type capacity() const { return mCapacity; }
  bool empty() const { return begin() == end(); }

  reference operator[](size_t i) { return *(begin() + i); }
  const_reference operator[](size_t i) const { return *(cbegin() + i); }

  pointer data() { return mBegin; }
  const_pointer data() const { return mBegin; }
  const_pointer cdata() const { return mBegin; }

  template <class... Args>
  void emplace_back(Args&&... args) {
    grow_for_size(size() + 1);
    new (mEnd) T(std::forward<Args>(args)...);
    ++mEnd;
  }

  void push_back(const T& t) { emplace_back(t); }
  void push_back(T&& t) { emplace_back(std::move(t)); }

  void clear() {
    destruct(begin(), end());
    mEnd = mBegin;
  }

  void reserve(size_type newCap) {
    if (newCap <= this->capacity()) {
      return;
    }
    set_capacity(newCap);
  }

  void resize(size_type newSize) { resize_impl<true>(newSize); }

  // This version of resizing doesn't initialize the newly allocated elements
  // Useful for the cases when value-initialization is noticeably slow and
  // one wants to directly construct or memcpy the elements into the resized
  // place.
  void resize_noinit(size_type newSize) { resize_impl<false>(newSize); }

  // Returns if the current vector's buffer is dynamically allocated.
  bool isAllocated() const { return this->cbegin() != smallBufferStart(); }

 protected:
  // Hide the default constructor so only SmallFixedVector can be
  // instantiated.
  SmallVector() = default;

  // Destroy all elements in the vector and free the array if it was allocated
  // dynamically.
  void dtor() {
    this->destruct(this->begin(), this->end());
    if (isAllocated()) {
      free(this->mBegin);
    }
  }

  // Just a convenience setter function to init all members at once.
  void init(iterator begin, iterator end, size_type capacity) {
    this->mBegin = begin;
    this->mEnd = end;
    this->mCapacity = capacity;
  }

  // An implementation of different resizing versions.
  template <bool init>
  void resize_impl(size_type newSize) {
    if (newSize < this->size()) {
      const auto newEnd = this->begin() + newSize;
      this->destruct(newEnd, this->end());
      this->mEnd = newEnd;
    } else if (newSize > this->size()) {
      grow_for_size(newSize);
      const auto newEnd = this->begin() + newSize;
      if (init) {
        std::uninitialized_fill(this->end(), newEnd, T());
      }
      this->mEnd = newEnd;
    }
  }

  // Templated append operation for a range of elements.
  template <class Iter>
  void insert_back(Iter b, Iter e) {
    if (b == e) {
      return;
    }
    const auto newSize = this->size() + (e - b);
    grow_for_size(newSize);
    this->mEnd = std::uninitialized_copy(b, e, this->mEnd);
  }

  // Multiplicative grow for the internal array so it can hold |newSize|
  // elements.
  // Doesn't change size(), only capacity().
  void grow_for_size(size_type newSize) {
    // Grow by 1.5x by default.
    if (newSize > capacity()) {
      set_capacity(std::max(newSize, capacity() + capacity() / 2));
    }
  }

  // Sets the capacity() to be exacly |newCap|. Allocates the array
  // dynamically, moves all elements over and (potentially) deallocates the
  // old array.
  // Doesn't change size(), only capacity().
  void set_capacity(size_type newCap) {
    // Here we can only be switching to the dynamic vector, as static one
    // always has its capacity on the maximum.
    const auto newBegin = static_cast<T*>(malloc(sizeof(T) * newCap));
    if (!newBegin) {
      abort();  // what else can we do here?
    }
    const auto newEnd =
        std::uninitialized_copy(std::make_move_iterator(this->begin()),
                                std::make_move_iterator(this->end()), newBegin);
    dtor();
    this->mBegin = newBegin;
    this->mEnd = newEnd;
    this->mCapacity = newCap;
  }

  // A convenience function to call destructor for a range of elements.
  static void destruct(T* b, T* e) {
    if (!std::is_trivially_destructible<T>::value) {
      for (; b != e; ++b) {
        b->~T();
      }
    }
  }

  // By design of the class, SmallFixedVector<> will be inheriting from
  // SmallVector<>, so its in-place storage array is going to be the very next
  // member after the last one here.
  // This function returns that address, and SmallFixedVector<> has a static
  // assert to make sure it remains correct.
  constexpr const void* smallBufferStart() const {
    return static_cast<const void*>(&mCapacity + 1);
  }

  // Standard set of members for a vector - begin, end and capacity.
  // These point to the currently used chunk of memory, no matter if it's a
  // heap-allocated one or an in-place array.
  iterator mBegin;
  iterator mEnd;
  size_type mCapacity;
};

// The implementation of a SmallVector with a fixed in-place size, |SmallSize|.
template <class T, size_t SmallSize>
class SmallFixedVector : public SmallVector<T> {
  using base = SmallVector<T>;

 public:
  // Grab these from the base class.
  using value_type = typename base::value_type;
  using iterator = typename base::iterator;
  using const_iterator = typename base::const_iterator;
  using pointer = typename base::pointer;
  using const_pointer = typename base::const_pointer;
  using reference = typename base::reference;
  using const_reference = typename base::const_reference;
  using size_type = typename base::size_type;

  static constexpr size_type kSmallSize = SmallSize;

  // Default constructor - set up an empty vector with capacity at full
  // internal array size.
  SmallFixedVector() {
    init_inplace();
  }

  // Ctor from a range of iterators
  template <class Iter>
  SmallFixedVector(Iter b, Iter e) : SmallFixedVector() {
    this->insert_back(b, e);
  }

  // Ctor from a range - anything that has begin and end.
  // Note: template constructor is never a copy/move-ctor.
  template <class Range, class = enable_if_c<!std::is_same<Range, T>::value &&
                                             is_range<Range>::value>>
  explicit SmallFixedVector(const Range& r)
      : SmallFixedVector(std::begin(r), std::end(r)) {}
  template <class Range, class = enable_if_c<!std::is_same<Range, T>::value &&
                                             is_range<Range>::value>>
  explicit SmallFixedVector(Range&& r)
      : SmallFixedVector(std::make_move_iterator(std::begin(r)),
                         std::make_move_iterator(std::end(r))) {}
  template <class U, class = enable_if_convertible<U, T>>
  SmallFixedVector(std::initializer_list<U> list)
      : SmallFixedVector(std::begin(list), std::end(list)) {}

  SmallFixedVector(const SmallFixedVector& other)
      : SmallFixedVector(other.begin(), other.end()) {}

  SmallFixedVector(SmallFixedVector&& other) {
    if (other.isAllocated()) {
      // Just steal the allocated memory from the |other|.
      this->mBegin = other.mBegin;
      this->mEnd = other.mEnd;
      this->mCapacity = other.mCapacity;
      other.init_inplace();
    } else {
      // Have to move individual elements.
      this->mBegin = mData.array;
      this->mEnd = std::uninitialized_copy(
          std::make_move_iterator(other.begin()),
          std::make_move_iterator(other.end()), this->begin());
      this->mCapacity = kSmallSize;
    }
  }

  SmallFixedVector& operator=(const SmallFixedVector& other) {
    if (&other != this) {
      this->clear();
      this->insert_back(other.begin(), other.end());
    }
    return *this;
  }

  SmallFixedVector& operator=(SmallFixedVector&& other) {
    if (other.isAllocated()) {
      // Steal it and we're done.
      this->dtor();
      this->mBegin = other.mBegin;
      this->mEnd = other.mEnd;
      this->mCapacity = other.mCapacity;
      other.init_inplace();
      return *this;
    }

    if (this->isAllocated() && this->mCapacity < other.size()) {
      // Not enough dynamic memory, switch to in-place.
      this->dtor();
      init_inplace();
    } else {
      // This could potentially be improved by move-assigning
      // only needed items and destroying the rest, but
      // destroy-all+construct-all is just simpler. For PODs it actually
      // is even faster as it's always a single memcpy().
      this->destruct(this->begin(), this->end());
    }

    // Move the whole |other| into the pre-cleaned memory
    const auto newEnd = std::uninitialized_copy(
        std::make_move_iterator(other.begin()),
        std::make_move_iterator(other.end()), this->mBegin);
    this->mEnd = newEnd;
    // |other| is valid as-is.
    return *this;
  }

  // Make sure we don't end up trying to move from an interface - it's just
  // inefficient with the current code.
  SmallFixedVector(base&& other) = delete;
  SmallFixedVector& operator=(base&& other) = delete;

 private:
  // A shortcut for initialization for in-place storage.
  void init_inplace() { this->init(mData.array, mData.array, kSmallSize); }

  // A union with empty constructor and destructor makes sure that the array
  // elements are not default-constructed in ctor and not destructed in dtor:
  // the class needs to be able manage their lifetime more precisely.
  union Data {
    alignas(size_type) T array[kSmallSize];

    Data() {}
    ~Data() {}
  } mData;
};

}  // namespace common
}  // namespace anbox
