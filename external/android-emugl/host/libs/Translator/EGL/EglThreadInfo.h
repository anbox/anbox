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
#ifndef EGL_THREAD_INFO_H
#define EGL_THREAD_INFO_H

#include <EGL/egl.h>

// Model thread-specific EGL state.
class EglThreadInfo {
public:
    // Return the thread-specific instance of EglThreadInfo to the caller.
    // The instance will be created on first call, and destroyed automatically
    // when the thread exits.
    static EglThreadInfo* get(void) __attribute__((const));

    // Set the error code |err|. Default value is EGL_NO_ERROR.
    void setError(EGLint err) { m_err = err; }

    // Get the error code. Note that this doesn't change the value,
    // unlike eglGetError() which resets it to EGL_NO_ERROR.
    EGLint getError() const { return m_err; }

    // Change the |api| value. Default is EGL_OPENGL_ES
    void setApi(EGLenum api) { m_api = api; }

    // Get the current API value.
    EGLenum getApi() const { return m_api; }

private:
    EglThreadInfo();

    EGLint m_err;
    EGLenum m_api;
};

#endif
