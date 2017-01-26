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

#ifndef EMUGL_COMMON_LOGGING_H_
#define EMUGL_COMMON_LOGGING_H_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wredundant-decls"

namespace emugl {
enum class LogLevel {
  TRACE,
  DEBUG,
  INFO,
  WARNING,
  ERROR,
  FATAL,
};
} // namespace emugl

typedef void (*logger_t)(const emugl::LogLevel &level, const char* fmt, ...);
extern logger_t emugl_logger;
extern logger_t emugl_cxt_logger;
void set_emugl_logger(logger_t f);
void set_emugl_cxt_logger(logger_t f);

#define GL_LOGGING 1

#if GL_LOGGING

#define GL_LOG(...) do { \
    emugl_logger(__VA_ARGS__); \
} while (0)

#define GL_CXT_LOG(...) do { \
    emugl_cxt_logger(__VA_ARGS__); \
} while (0)

#else
#define GL_LOG(...) 0
#endif

#pragma GCC diagnostic pop

#endif
