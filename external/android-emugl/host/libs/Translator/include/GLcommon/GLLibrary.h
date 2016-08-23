// Copyright (C) 2015 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GLCOMMON_GL_LIBRARY_H
#define GLCOMMON_GL_LIBRARY_H

// An interface class used to model the desktop GL library to use at runtime.
// Use GlLibrary::getHostInstance() to retrieve the instance corresponding
// to the host's default system GL library.
class GlLibrary {
public:
    // Generic pointer-to-function type.
    typedef void (*GlFunctionPointer)(void);

    // Empty constructor.
    GlLibrary() {}

    // Empty destructor.
    virtual ~GlLibrary() {}

    // Find the function named |name| in the library and return its address.
    virtual GlFunctionPointer findSymbol(const char* name) = 0;
};

#endif  // GLCOMMON_GL_LIBRARY_H
