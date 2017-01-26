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

#include "emugl/common/logging.h"

void default_logger(const emugl::LogLevel &level, const char* fmt, ...) { }

logger_t emugl_logger = default_logger;
logger_t emugl_cxt_logger = default_logger;

void set_emugl_logger(logger_t f) {
    if (!f) {
        emugl_logger = default_logger;
    } else {
        emugl_logger = f;
    }
}

void set_emugl_cxt_logger(logger_t f) {
    if (!f) {
        emugl_cxt_logger = default_logger;
    } else {
        emugl_cxt_logger = f;
    }
}
