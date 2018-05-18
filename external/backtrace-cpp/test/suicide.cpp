/*
 * test/suicide.cpp
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

#include "backward.hpp"

#include <cstdio>
#include <sys/resource.h>
#include "test/test.hpp"

using namespace backward;

void badass_function()
{
	char* ptr = (char*)42;
	*ptr = 42;
}

TEST_SEGFAULT (invalid_write)
{
	badass_function();
}

int you_shall_not_pass()
{
	char* ptr = (char*)42;
	int v = *ptr;
	return v;
}

TEST_SEGFAULT(invalid_read)
{
	int v = you_shall_not_pass();
	std::cout << "v=" << v << std::endl;
}

void abort_abort_I_repeat_abort_abort()
{
	std::cout << "Jumping off the boat!" << std::endl;
	abort();
}

TEST_ABORT (calling_abort)
{
	abort_abort_I_repeat_abort_abort();
}

volatile int zero = 0;

int divide_by_zero()
{
	std::cout << "And the wild black hole appears..." << std::endl;
	int v = 42 / zero;
	return v;
}

TEST_DIVZERO (divide_by_zero)
{
	int v = divide_by_zero();
	std::cout << "v=" << v << std::endl;
}

int bye_bye_stack(int i) {
	return bye_bye_stack(i + 1) + bye_bye_stack(i * 2);
}

TEST_SEGFAULT(stackoverflow)
{
	struct rlimit limit;
	limit.rlim_max = 8096;
	setrlimit(RLIMIT_STACK, &limit);
	int r = bye_bye_stack(42);
	std::cout << "r=" << r << std::endl;
}
