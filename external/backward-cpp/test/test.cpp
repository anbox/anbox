/*
 * test/test.cpp
 * Copyright 2013 Google Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "test/test.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

TEST(empty_test) {}

TEST_FAIL_ASSERT(fail_assert) { ASSERT(1 == 2); }

TEST_FAIL_ASSERT(fail_assert_ge) { ASSERT_GE(4, 5); }

TEST_UNCAUGHT_EXCEPTION(uncaught_exception) {
  throw std::runtime_error("some random runtime error");
}

TEST_UNCAUGHT_EXCEPTION(uncaught_exception_int) { throw 42; }

TEST_SEGFAULT(segfault) {
  char *a = 0;
  char b = a[42];
  std::cout << "result: " << b << std::endl;
}

TEST_ABORT(abort) { abort(); }

TEST(catch_int) {
  ASSERT_THROW({ throw 42; }, int);
}

TEST_FAIL_ASSERT(fail_catch_int) { ASSERT_THROW({}, int); }

TEST_FAIL_ASSERT(fail_no_throw) {
  ASSERT_NO_THROW({ throw 42; });
}

TEST(any_throw) {
  ASSERT_ANY_THROW({ throw 42; });
}

TEST_FAIL_ASSERT(fail_any_throw) { ASSERT_ANY_THROW({}); }
