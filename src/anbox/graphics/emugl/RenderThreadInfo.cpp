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

#include "anbox/graphics/emugl/RenderThreadInfo.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#include "external/android-emugl/shared/emugl/common/lazy_instance.h"
#include "external/android-emugl/shared/emugl/common/thread_store.h"
#pragma GCC diagnostic pop

namespace {
class ThreadInfoStore : public ::emugl::ThreadStore {
 public:
  ThreadInfoStore() : ::emugl::ThreadStore(NULL) {}
};
}

static ::emugl::LazyInstance<ThreadInfoStore> s_tls = LAZY_INSTANCE_INIT;

RenderThreadInfo::RenderThreadInfo() { s_tls->set(this); }

RenderThreadInfo::~RenderThreadInfo() { s_tls->set(NULL); }

RenderThreadInfo* RenderThreadInfo::get() {
  return static_cast<RenderThreadInfo*>(s_tls->get());
}
