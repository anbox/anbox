/*
 * _test_main.cpp
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

#include "test.hpp"
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#define strcasecmp _stricmp
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

#if defined(__has_include) && __has_include(<error.h>)
#include <error.h>
#else
#include <stdarg.h>

#ifdef _WIN32
char argv0[MAX_PATH];
inline const char *getprogname() {
  return GetModuleFileName(NULL, argv0, sizeof(argv0)) ? argv0 : NULL;
}
#elif !defined(__APPLE__)
// N.B.  getprogname() is an Apple/BSD-ism.
// program_invocation_name is a GLIBC-ism, but it's also
//  supported by libmusl.
#define getprogname() program_invocation_name
#endif

void error(int status, int errnum, const char *format, ...) {
  fflush(stdout);
  fprintf(stderr, "%s: ", getprogname());

  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  if (errnum != 0) {
    fprintf(stderr, ": %s\n", strerror(errnum));
  } else {
    fprintf(stderr, "\n");
  }
  if (status != 0) {
    exit(status);
  }
}
#endif

using namespace test;

bool run_test(TestBase &test, bool use_child_process = true) {
  if (!use_child_process) {
    exit(static_cast<int>(test.run()));
  }

  printf("-- running test case: %s\n", test.name);

  fflush(stdout);

  test::TestStatus status = test::SUCCESS;

#ifdef _WIN32
  char filename[256];
  GetModuleFileName(NULL, filename, 256); // TODO: check for error
  std::string cmd_line = filename;
  cmd_line += " --nofork ";
  cmd_line += test.name;

  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  if (!CreateProcessA(nullptr, const_cast<char *>(cmd_line.c_str()), nullptr,
                      nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
    printf("unable to create process\n");
    exit(-1);
  }

  WaitForSingleObject(pi.hProcess, INFINITE);

  DWORD exit_code;
  GetExitCodeProcess(pi.hProcess, &exit_code);
  switch (exit_code) {
  case 3:
    status = test::SIGNAL_ABORT;
    break;
  case 5:
    status = test::EXCEPTION_UNCAUGHT;
    break;
  case EXCEPTION_ACCESS_VIOLATION:
    status = test::SIGNAL_SEGFAULT;
    break;
  case EXCEPTION_STACK_OVERFLOW:
    status = test::SIGNAL_SEGFAULT;
    break;
  case EXCEPTION_INT_DIVIDE_BY_ZERO:
    status = test::SIGNAL_DIVZERO;
    break;
  }
  printf("Exit code: %lu\n", exit_code);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  if (test.expected_status == test::ASSERT_FAIL) {
    // assert calls abort on windows
    return (status & test::SIGNAL_ABORT);
  }

#else

  pid_t child_pid = fork();
  if (child_pid == 0) {
    exit(static_cast<int>(test.run()));
  }
  if (child_pid == -1) {
    error(EXIT_FAILURE, 0, "unable to fork");
  }

  int child_status = 0;
  waitpid(child_pid, &child_status, 0);

  if (WIFEXITED(child_status)) {
    int exit_status = WEXITSTATUS(child_status);
    if (exit_status & ~test::STATUS_MASK) {
      status = test::FAILED;
    } else {
      status = static_cast<test::TestStatus>(exit_status);
    }
  } else if (WIFSIGNALED(child_status)) {
    const int signum = WTERMSIG(child_status);
    printf("!! signal (%d) %s\n", signum, strsignal(signum));
    switch (signum) {
    case SIGABRT:
      status = test::SIGNAL_ABORT;
      break;
    case SIGSEGV:
    case SIGBUS:
      status = test::SIGNAL_SEGFAULT;
      break;
    case SIGFPE:
      status = test::SIGNAL_DIVZERO;
      break;
    default:
      status = test::SIGNAL_UNCAUGHT;
    }
  }

#endif

  if (test.expected_status == test::FAILED) {
    return (status & test::FAILED);
  }

  if (test.expected_status == test::SIGNAL_UNCAUGHT) {
    return (status & test::SIGNAL_UNCAUGHT);
  }

  return status == test.expected_status;
}

int main(int argc, const char *const argv[]) {

#ifdef _WIN32
  _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif

  if (argc == 3 && strcmp("--nofork", argv[1]) == 0) {
    // Windows has no fork, so we simulate it
    // we only execute one test, without forking
    for (test_registry_t::iterator it = test_registry().begin();
         it != test_registry().end(); ++it) {
      TestBase &test = **it;
      if (strcasecmp(argv[2], test.name) == 0) {
        run_test(test, false);

        return 0;
      }
    }
    return -1;
  }

  size_t success_cnt = 0;
  size_t total_cnt = 0;
  for (test_registry_t::iterator it = test_registry().begin();
       it != test_registry().end(); ++it) {
    TestBase &test = **it;

    bool consider_test = (argc <= 1);
    for (int i = 1; i < argc; ++i) {
      if (strcasecmp(argv[i], test.name) == 0) {
        consider_test = true;
        break;
      }
    }
    if (!consider_test) {
      continue;
    }

    total_cnt += 1;
    if (run_test(test)) {
      printf("-- test case success: %s\n", test.name);
      success_cnt += 1;
    } else {
      printf("** test case FAILED : %s\n", test.name);
    }
  }
  printf("-- tests passing: %zu/%zu", success_cnt, total_cnt);
  if (total_cnt) {
    printf(" (%zu%%)\n", success_cnt * 100 / total_cnt);
  } else {
    printf("\n");
  }
  return (success_cnt == total_cnt) ? EXIT_SUCCESS : EXIT_FAILURE;
}
