/*
 * test/segfault.cpp
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

#include <stdio.h>
#include "test/test.hpp"

using namespace backward;

void badass_function() {
	char* ptr = (char*)42;
	*ptr = 42;
}

TEST_SEGFAULT (pprint_sigsev) {
	std::vector<int> signals;
	signals.push_back(SIGSEGV);
	SignalHandling sh(signals);
	std::cout << std::boolalpha << "sh.loaded() == " << sh.loaded() << std::endl;
	badass_function();
}

TEST_SEGFAULT (wont_pprint) {
	std::vector<int> signals;
	signals.push_back(SIGABRT);
	SignalHandling sh(signals);
	std::cout << std::boolalpha << "sh.loaded() == " << sh.loaded() << std::endl;
	badass_function();
}
