// Copyright (C) 2016 The Android Open Source Project
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

#include "anbox/common/scope_ptr.h"

#include <gtest/gtest.h>

#include <type_traits>
#include <utility>

namespace anbox {
namespace common {

TEST(ScopedPtr, FuncDelete_conversions) {
    // This test makes sure the code compiles, it doesn't do any runtime checks.
    auto lambda = [](int i) { return i; };
    FuncDelete<decltype(lambda)> lambdaFd(lambda);
    // unary + converts a captureless lambda into a raw function pointer
    FuncDelete<decltype(+lambda)> funcFd1(+lambda);

    // copy ctor
    FuncDelete<decltype(+lambda)> funcFd2(funcFd1);
    // assignment operator
    funcFd2 = funcFd1;
    // move operator
    funcFd2 = std::move(funcFd1);

    // conversion ctor
    FuncDelete<decltype(+lambda)> funcFd3(lambdaFd);
    // conversion move ctor
    FuncDelete<decltype(+lambda)> funcFd4(std::move(lambdaFd));
    // conversion assignment
    funcFd3 = lambdaFd;
    // conversion move
    funcFd3 = std::move(lambdaFd);
}

TEST(ScopedPtr, makeCustomScopedPtr_fromLambda) {
    auto freeAsLambda = [](void* ptr) { free(ptr); };

    auto ptr1 = makeCustomScopedPtr(malloc(1), freeAsLambda);

    ScopedPtr<void, FuncDelete<void(*)(void*)>> ptr2 =
            makeCustomScopedPtr(malloc(1), freeAsLambda);

    ScopedPtr<void, FuncDelete<void(*)(void*)>> ptr3 =
            makeCustomScopedPtr(malloc(1), +freeAsLambda);

    static_assert(!std::is_same<decltype(ptr1), decltype(ptr2)>::value,
                  "Custom ScopedPtr<> from a lambda expression type may not "
                  "be the same as with a function pointer");
}
}  // namespace common
}  // namespace anbox
