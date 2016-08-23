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
#include "EglThreadInfo.h"
#include "EglOsApi.h"

#include "emugl/common/lazy_instance.h"
#include "emugl/common/thread_store.h"

namespace {

class EglThreadInfoStore : public emugl::ThreadStore {
public:
    EglThreadInfoStore() : emugl::ThreadStore(&destructor) {}
private:
    static void destructor(void* value) {
        delete static_cast<EglThreadInfo*>(value);
    }
};

}  // namespace

EglThreadInfo::EglThreadInfo() :
        m_err(EGL_SUCCESS), m_api(EGL_OPENGL_ES_API) {}

static emugl::LazyInstance<EglThreadInfoStore> s_tls = LAZY_INSTANCE_INIT;

EglThreadInfo* EglThreadInfo::get(void)
{
    EglThreadInfo *ti = static_cast<EglThreadInfo*>(s_tls->get());
    if (!ti) {
        ti = new EglThreadInfo();
        s_tls->set(ti);
    }
    return ti;
}

