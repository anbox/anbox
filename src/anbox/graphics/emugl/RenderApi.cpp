/*
* Copyright (C) 2011-2015 The Android Open Source Project
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

#include "RenderApi.h"

#include "DispatchTables.h"

#include "OpenGLESDispatch/EGLDispatch.h"
#include "OpenGLESDispatch/GLESv1Dispatch.h"
#include "OpenGLESDispatch/GLESv2Dispatch.h"

#include "emugl/common/crash_reporter.h"
#include "emugl/common/logging.h"

#include <string.h>

GLESv2Dispatch s_gles2;
GLESv1Dispatch s_gles1;

namespace anbox {
namespace graphics {
namespace emugl {
bool initialize(emugl_logger_struct log_funcs, emugl_crash_func_t crash_func) {
  set_emugl_crash_reporter(crash_func);
  set_emugl_logger(log_funcs.coarse);
  set_emugl_cxt_logger(log_funcs.fine);

  if (!init_egl_dispatch()) return false;

  if (!gles1_dispatch_init(&s_gles1)) return false;

  if (!gles2_dispatch_init(&s_gles2)) return false;

  return true;
}
}  // namespace emugl
}  // namespace graphics
}  // namespace anbox
