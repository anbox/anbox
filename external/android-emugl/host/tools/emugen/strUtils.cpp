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
#include "strUtils.h"

using namespace std;


std::string getNextToken(const std::string & str, size_t pos, size_t * last, const std::string & delim)
{
    if (str.size() == 0 || pos >= str.size()) return "";

    pos = str.find_first_not_of(WHITESPACE, pos);
    if (pos == std::string::npos) return "";

    *last = str.find_first_of(delim, pos);
    if (*last == std::string::npos) *last = str.size();
    std::string retval = str.substr(pos, *last - pos);
    retval = trim(retval);
    return retval;
}


std::string trim(const string & str)
{
  string result;
  string::size_type start = str.find_first_not_of(WHITESPACE, 0);
  string::size_type end = str.find_last_not_of(WHITESPACE);
  if (start == string::npos || end == string::npos) {
    result = string("");
  } else {
    result = str.substr(start, end - start + 1);
  }
  return result;
}


