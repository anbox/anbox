/*
* Copyright (C) 2016 The Android Open Source Project
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

#include "crash_reporter.h"

#include <cstdlib>

void default_crash_reporter(const emugl::LogLevel &level, const char* format, ...) {
    abort();
}

logger_t emugl_crash_reporter = default_crash_reporter;

void set_emugl_crash_reporter(logger_t crash_reporter) {
    if (crash_reporter) {
        emugl_crash_reporter = crash_reporter;
    } else {
        emugl_crash_reporter = default_crash_reporter;
    }
}
