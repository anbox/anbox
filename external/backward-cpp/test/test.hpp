/*
 * test/test.hpp
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

#pragma once
#ifndef H_54E531F7_9154_454B_BEB9_257408429470
#define H_54E531F7_9154_454B_BEB9_257408429470

#include <cstdio>
#include <cstring>
#include <exception>
#include <sstream>
#include <string>
#include <vector>

namespace test {

struct AssertFailedError : std::exception {
  ~AssertFailedError() throw() {}

  AssertFailedError(const char *filename, int _line, const char *_errmsg)
      : basename(_basename(filename)), line(_line), errmsg(_errmsg) {}

  const char *what() const throw() {
    if (!_what.size()) {
      std::ostringstream ss;
      ss << "assertion failed (" << basename << ":" << line;
      ss << ") " << errmsg;
      _what = ss.str();
    }
    return _what.c_str();
  }

  const char *basename;
  int line;
  const char *errmsg;

  mutable std::string _what;

  static const char *_basename(const char *filename) {
    const char *basename = filename + strlen(filename);
    while (basename != filename && *basename != '/') {
      basename -= 1;
    }
    return basename + 1;
  }
};

enum TestStatus {
  SUCCESS = 0 << 0,
  FAILED = 1 << 0,

  ASSERT_FAIL = FAILED | 1 << 1,
  EXCEPTION_UNCAUGHT = FAILED | 2 << 1,
  SIGNAL_UNCAUGHT = FAILED | 3 << 1,
  SIGNAL_SEGFAULT = SIGNAL_UNCAUGHT | 1 << 3,
  SIGNAL_ABORT = SIGNAL_UNCAUGHT | 2 << 3,
  SIGNAL_DIVZERO = SIGNAL_UNCAUGHT | 2 << 3,

  STATUS_MASK = 0x1F
};

struct TestBase {
  const char *name;
  TestStatus expected_status;

  virtual ~TestBase() {}
  TestBase(const char *, TestStatus);
  virtual void do_test() = 0;

  TestStatus run() {
    try {
      do_test();
      return SUCCESS;
    } catch (const AssertFailedError &e) {
      printf("!! %s\n", e.what());
      return ASSERT_FAIL;
    } catch (const std::exception &e) {
      printf("!! exception: %s\n", e.what());
      return EXCEPTION_UNCAUGHT;
    } catch (...) {
      printf("!! unknown exception\n");
      return EXCEPTION_UNCAUGHT;
    }
  }
};

typedef std::vector<TestBase *> test_registry_t;
inline test_registry_t &test_registry() {
  static test_registry_t reg;
  return reg;
}

inline TestBase::TestBase(const char *n, TestStatus s)
    : name(n), expected_status(s) {
  test_registry().push_back(this);
}

} // namespace test

#define _TEST_STATUS(name, status)                                             \
  struct TEST_##name : ::test::TestBase {                                      \
    TEST_##name() : TestBase(#name, status) {}                                 \
    void do_test();                                                            \
  } TEST_##name;                                                               \
  void TEST_##name::do_test()

#define TEST(name) _TEST_STATUS(name, ::test::SUCCESS)
#define TEST_FAIL(name) _TEST_STATUS(name, ::test::FAILED)
#define TEST_FAIL_ASSERT(name) _TEST_STATUS(name, ::test::ASSERT_FAIL)
#define TEST_UNCAUGHT_EXCEPTION(name)                                          \
  _TEST_STATUS(name, ::test::EXCEPTION_UNCAUGHT)
#define TEST_UNCAUGHT_SIGNAL(name) _TEST_STATUS(name, ::test::SIGNAL_UNCAUGHT)
#define TEST_SEGFAULT(name) _TEST_STATUS(name, ::test::SIGNAL_SEGFAULT)
#define TEST_ABORT(name) _TEST_STATUS(name, ::test::SIGNAL_ABORT)
#define TEST_DIVZERO(name) _TEST_STATUS(name, ::test::SIGNAL_DIVZERO)

#define ASSERT(expr)                                                           \
  (expr) ? static_cast<void>(0)                                                \
         : throw ::test::AssertFailedError(__FILE__, __LINE__, #expr)

#define _ASSERT_BINOP(a, b, cmp)                                               \
  (!(a cmp b)) ? static_cast<void>(0)                                          \
               : throw ::test::AssertFailedError(                              \
                     __FILE__, __LINE__, "because " #a " " #cmp " " #b)

#define ASSERT_EQ(a, b) _ASSERT_BINOP(a, b, !=)
#define ASSERT_NE(a, b) _ASSERT_BINOP(a, b, ==)
#define ASSERT_LT(a, b) _ASSERT_BINOP(a, b, >=)
#define ASSERT_LE(a, b) _ASSERT_BINOP(a, b, >)
#define ASSERT_GT(a, b) _ASSERT_BINOP(a, b, <=)
#define ASSERT_GE(a, b) _ASSERT_BINOP(a, b, <)

#define ASSERT_THROW(expr, e_type)                                             \
  do {                                                                         \
    try {                                                                      \
      expr                                                                     \
    } catch (const e_type &) {                                                 \
      break;                                                                   \
    }                                                                          \
    throw ::test::AssertFailedError(__FILE__, __LINE__,                        \
                                    "expected exception " #e_type);            \
  } while (0)

#define ASSERT_ANY_THROW(expr)                                                 \
  do {                                                                         \
    try {                                                                      \
      expr                                                                     \
    } catch (...) {                                                            \
      break;                                                                   \
    }                                                                          \
    throw ::test::AssertFailedError(__FILE__, __LINE__,                        \
                                    "expected any exception");                 \
  } while (0)

#define ASSERT_NO_THROW(expr)                                                  \
  try {                                                                        \
    expr                                                                       \
  } catch (...) {                                                              \
    throw ::test::AssertFailedError(__FILE__, __LINE__,                        \
                                    "no exception expected");                  \
  }

#endif /* H_GUARD */
