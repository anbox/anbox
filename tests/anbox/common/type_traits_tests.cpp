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

#include "anbox/common/type_traits.h"

#include <gtest/gtest.h>

#include <array>
#include <functional>
#include <list>
#include <vector>

namespace anbox {
namespace common {
TEST(TypeTraits, IsCallable) {
    class C;
    C* c = nullptr;

    auto lambda = [c](bool) -> C* { return nullptr; };

    static_assert(is_callable_as<void(), void()>::value, "simple function");
    static_assert(is_callable_as<void(&)(), void()>::value, "function reference");
    static_assert(is_callable_as<void(*)(), void()>::value, "function pointer");
    static_assert(is_callable_as<int(C&, C*), int(C&, C*)>::value,
                  "function with arguments and return type");
    static_assert(is_callable_as<decltype(lambda), C*(bool)>::value, "lambda");
    static_assert(is_callable_as<std::function<bool(int)>, bool(int)>::value,
                  "std::function");

    static_assert(!is_callable_as<int, void()>::value, "int should not be callable");
    static_assert(!is_callable_as<C, void()>::value, "incomplete type");
    static_assert(!is_callable_as<void(), void(int)>::value, "different arguments");
    static_assert(!is_callable_as<int(), void()>::value, "different return types");
    static_assert(!is_callable_as<int(), short()>::value,
                  "slightly different return types");
    static_assert(!is_callable_as<int(int), int(int, int)>::value,
                  "more arguments");
    static_assert(!is_callable_as<int(int, int), int(int)>::value,
                  "less arguments");

    static_assert(!is_callable_as<int(int), int>::value,
                  "bad required signature");
}

TEST(TypeTraits, IsTemplateInstantiation) {
    static_assert(!is_template_instantiation_of<int, std::vector>::value,
                  "int is not an instance of vector");
    static_assert(!is_template_instantiation_of<std::list<std::vector<int>>, std::vector>::value,
                  "list is not an instance of vector");

    static_assert(is_template_instantiation_of<std::vector<int>, std::vector>::value,
                  "std::vector<int> is an instance of vector");
    static_assert(is_template_instantiation_of<std::vector<std::vector<std::vector<int>>>, std::vector>::value,
                  "nested std::vector<> is an instance of vector");
}

TEST(TypeTraits, IsRange) {
    static_assert(is_range<std::vector<int>>::value,
                  "vector<> should be detected as a range");
    static_assert(is_range<const std::list<std::function<void()>>>::value,
                  "const list<> should be detected as a range");
    static_assert(is_range<std::array<std::vector<int>, 10>>::value,
                  "array<> should be detected as a range");
    char arr[100];
    static_assert(is_range<decltype(arr)>::value,
                  "C array should be detected as a range");
    static_assert(is_range<decltype("string")>::value,
                  "String literal should be detected as a range");

    static_assert(!is_range<int>::value, "int shouldn't be a range");
    static_assert(!is_range<int*>::value, "int* shouldn't be a range");
    static_assert(!is_range<const int*>::value,
                  "even const int* shouldn't be a range");
}
}  // namespace common
}  // namespace anbox
