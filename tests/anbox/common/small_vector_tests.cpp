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

#include "anbox/common/small_vector.h"
#include "anbox/common/scope_ptr.h"
#include "anbox/testing/gtest_utils.h"

#include <gtest/gtest.h>

#include <string>

namespace anbox {
namespace common {
TEST(SmallVector, basic) {
    SmallFixedVector<char, 10> sv;
    EXPECT_EQ(sv.size(), 0);
    EXPECT_TRUE(sv.empty());

    EXPECT_EQ(10, sv.capacity());
    EXPECT_FALSE(sv.isAllocated());

    const int values[] = {1, 2, 3};
    sv = SmallFixedVector<char, 10>(values, values + 3);
    EXPECT_EQ(3, sv.size());
    EXPECT_FALSE(sv.empty());
    EXPECT_EQ(10, sv.capacity());
    EXPECT_FALSE(sv.isAllocated());
    EXPECT_TRUE(anbox::testing::RangesMatch(values, sv));

    sv.clear();
    EXPECT_EQ(0, sv.size());
    EXPECT_TRUE(sv.empty());

    const char str[] = "this is a long string for insertion";
    sv = SmallFixedVector<char, 10>(str);
    EXPECT_EQ(sizeof(str), sv.size());
    EXPECT_GT(sv.capacity(), 10);
    EXPECT_TRUE(sv.isAllocated());
    EXPECT_TRUE(anbox::testing::RangesMatch(str, sv));

    // make sure explicit loops over indices and iterators work
    for (size_t i = 0; i != sv.size(); ++i) {
        EXPECT_EQ(str[i], sv[i]);
    }
    const char* c = str;
    for (auto i : sv) {
        EXPECT_EQ(*c++, i);
    }
    c = str;
    for (auto it = sv.begin(); it != sv.end(); ++it, ++c) {
        EXPECT_EQ(*c, *it);
    }

    c = str;
    for (auto it = sv.data(); it != sv.data() + sv.size(); ++it, ++c) {
        EXPECT_EQ(*c, *it);
    }
}

TEST(SmallVector, ctor) {
    {
        SmallFixedVector<int, 1> sv;
        EXPECT_EQ(sv.size(), 0);
    }

    {
        const int values[] = {1, 2, 3};
        SmallFixedVector<int, 10> sv(values, values + 3);
        EXPECT_TRUE(anbox::testing::RangesMatch(values, sv));
    }

    {
        const int values[] = {1, 2, 3};
        SmallFixedVector<int, 10> sv(values);
        EXPECT_TRUE(anbox::testing::RangesMatch(values, sv));
    }

    {
        const int values[] = {1, 2, 3};
        SmallFixedVector<int, 10> sv(values);
        EXPECT_TRUE(anbox::testing::RangesMatch(values, sv));
    }

    {
        const int values[] = {1, 2, 3};
        SmallFixedVector<int, 10> sv = {1, 2, 3};
        EXPECT_TRUE(anbox::testing::RangesMatch(values, sv));
    }

    {
        const int values[] = {1, 2, 3};
        SmallFixedVector<int, 10> sv1(values);
        SmallFixedVector<int, 10> sv2(sv1);

        EXPECT_TRUE(anbox::testing::RangesMatch(values, sv1));
        EXPECT_TRUE(anbox::testing::RangesMatch(sv2, sv1));

        SmallFixedVector<int, 10> sv3(std::move(sv1));
        EXPECT_TRUE(anbox::testing::RangesMatch(sv3, values));
        // don't check |sv1| - it is not required to be empty.
    }
}

TEST(SmallVector, dtor) {
    // Count all destructor calls for the elements and make sure they're called
    // enough times.
    int destructedTimes = 0;
    auto deleter = [](int* p) { ++(*p); };
    auto item1 = makeCustomScopedPtr(&destructedTimes, deleter);
    auto item2 = makeCustomScopedPtr(&destructedTimes, deleter);
    auto item3 = makeCustomScopedPtr(&destructedTimes, deleter);

    {
        SmallFixedVector<decltype(item1), 1> sv;
        sv.emplace_back(std::move(item1));
        sv.emplace_back(std::move(item2));
        // this one is already empty, so it won't add to |destructedTimes|.
        sv.emplace_back(std::move(item2));
        sv.emplace_back(std::move(item3));
    }

    EXPECT_EQ(3, destructedTimes);
}

TEST(SmallVector, modifiers) {
    SmallFixedVector<unsigned, 5> sv;

    EXPECT_EQ(0, sv.size());
    EXPECT_EQ(5, sv.capacity());

    sv.reserve(4);
    EXPECT_EQ(0, sv.size());
    EXPECT_EQ(5, sv.capacity());
    EXPECT_FALSE(sv.isAllocated());

    sv.reserve(6);
    EXPECT_EQ(0, sv.size());
    EXPECT_EQ(6, sv.capacity());
    EXPECT_TRUE(sv.isAllocated());

    sv.resize(3);
    EXPECT_EQ(3, sv.size());
    EXPECT_EQ(6, sv.capacity());
    EXPECT_TRUE(sv.isAllocated());

    sv.resize(10);
    EXPECT_EQ(10, sv.size());
    EXPECT_GE(sv.capacity(), 10);
    EXPECT_TRUE(sv.isAllocated());

    sv.push_back(1);
    EXPECT_EQ(11, sv.size());
    EXPECT_GE(sv.capacity(), 11);

    sv.emplace_back(2);
    EXPECT_EQ(12, sv.size());
    EXPECT_GE(sv.capacity(), 12);

    sv.clear();
    EXPECT_EQ(0, sv.size());
    EXPECT_GE(sv.capacity(), 12);

    // resize_noinit() doesn't really have anything specific we can test
    // compared to resize()
    sv.resize_noinit(1);
    EXPECT_EQ(1, sv.size());
    EXPECT_GE(sv.capacity(), 12);

    sv.resize_noinit(100);
    EXPECT_EQ(100, sv.size());
    EXPECT_GE(sv.capacity(), 100);

    SmallFixedVector<std::string, 5> strings = {"a", "b", "c"};
    strings.emplace_back("d");
    strings.push_back("e");
    EXPECT_EQ(5, strings.size());
    EXPECT_EQ(5, strings.capacity());
    EXPECT_FALSE(strings.isAllocated());

    strings.push_back(std::string("e"));
    EXPECT_EQ(6, strings.size());
    EXPECT_GE(strings.capacity(), 6);
    EXPECT_TRUE(strings.isAllocated());
}

TEST(SmallVector, useThroughInterface) {
    SmallFixedVector<int, 10> sfv = {1, 2, 3};
    SmallVector<int>& sv = sfv;
    EXPECT_TRUE(anbox::testing::RangesMatch(sv, sfv));
    EXPECT_EQ(sv.isAllocated(), sfv.isAllocated());

    sv.reserve(20);
    EXPECT_TRUE(sv.isAllocated());
    EXPECT_EQ(sv.isAllocated(), sfv.isAllocated());

    // now make sure that deleting through base class cleans up the memory
    {
        int destructedTimes = 0;
        auto deleter = [](int* p) { ++(*p); };
        auto item1 = makeCustomScopedPtr(&destructedTimes, deleter);
        auto item2 = makeCustomScopedPtr(&destructedTimes, deleter);
        SmallVector<decltype(item1)>* sv =
                new SmallFixedVector<decltype(item1), 1>();
        sv->push_back(std::move(item1));
        sv->emplace_back(std::move(item2));
        delete sv;
        EXPECT_EQ(2, destructedTimes);
    }
}
}  // namespace common
}  // namespace anbox
