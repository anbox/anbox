/*
* Copyright (C) 2011 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef STR_UTILS_H_
#define STR_UTILS_H_

#include <string>
#include <sstream>

#define WHITESPACE " \t\n"

std::string trim(const std::string & str);
std::string getNextToken(const std::string & str, size_t pos, size_t * last, const std::string & delim);
template <class T> std::string inline toString(const T& t) {
    std::stringstream ss;
    ss << t;
    return ss.str();

}

#endif
