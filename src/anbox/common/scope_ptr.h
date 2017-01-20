// Copyright 2014 The Android Open Source Project
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

#include <memory>
#include <type_traits>
#include <utility>

#include <stdlib.h>

namespace anbox {
namespace common {

struct FreeDelete {
  template <class T>
  void operator()(T ptr) const {
    free(ptr);
  }
};

template <class Func>
struct FuncDelete {
  explicit FuncDelete(Func f = {}) : mF(f) {}

  FuncDelete(const FuncDelete& other) = default;
  FuncDelete(FuncDelete&& other) = default;
  FuncDelete& operator=(const FuncDelete& other) = default;
  FuncDelete& operator=(FuncDelete&& other) = default;

  // To be able to copy/move from all compatible template instantiations.
  template <class U>
  friend struct FuncDelete;

  // Template constructors and move assignment from compatible instantiations.
  template <class U>
  FuncDelete(const FuncDelete<U>& other) : mF(other.mF) {}
  template <class U>
  FuncDelete(FuncDelete<U>&& other) : mF(std::move(other.mF)) {}
  template <class U>
  FuncDelete& operator=(const FuncDelete<U>& other) {
    mF = other.mF;
    return *this;
  }
  template <class U>
  FuncDelete& operator=(FuncDelete<U>&& other) {
    mF = std::move(other.mF);
    return *this;
  }

  // This is the actual deleter call.
  template <class T>
  void operator()(T t) const {
    mF(t);
  }

 private:
  Func mF;
};

template <class T, class Deleter = std::default_delete<T>>
using ScopedPtr = std::unique_ptr<T, Deleter>;

template <class T>
using ScopedCPtr = std::unique_ptr<T, FreeDelete>;

template <class T, class Func>
using ScopedCustomPtr = std::unique_ptr<T, FuncDelete<Func>>;

// A factory function that creates a scoped pointer with |deleter|
// function used as a deleter - it is called at the scope exit.
// Note: enable_if<> limits the scope of allowed arguments to pointers and
//  std::nullptr_t (to allow makeCustomScopedPtr(nullptr, ...) calls).
template <class T,
          class Func,
          class = enable_if_c<std::is_same<T, std::nullptr_t>::value ||
                              std::is_pointer<T>::value>>
ScopedCustomPtr<
    typename std::decay<typename std::remove_pointer<T>::type>::type,
    typename std::decay<Func>::type>
makeCustomScopedPtr(T data, Func deleter) {
  return ScopedCustomPtr<
      typename std::decay<typename std::remove_pointer<T>::type>::type,
      typename std::decay<Func>::type>(
      data, FuncDelete<typename std::decay<Func>::type>(deleter));
}

}  // namespace common
}  // namespace anbox
