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

#include "EglOsApi.h"

#include "emugl/common/lazy_instance.h"

namespace {
class NullEngine : public EglOS::Engine {
public:
    NullEngine() = default;

    virtual EglOS::Display* getDefaultDisplay() override {
        return nullptr;
    }

    virtual GlLibrary* getGlLibrary() {
        return nullptr;
    }

    virtual EglOS::Surface* createWindowSurface(EGLNativeWindowType window) override {
        return nullptr;
    }
};

emugl::LazyInstance<NullEngine> sHostEngine = LAZY_INSTANCE_INIT;
}

#if 0
EglOS::Engine* EglOS::Engine::getHostInstance() {
    return sHostEngine.ptr();
}
#endif
