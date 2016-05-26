// Copyright 2016 The Android Open Source Project
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

#pragma once

#include "OpenGLESDispatch/gles_common_functions.h"
#include "OpenGLESDispatch/gles_extensions_functions.h"
#include "OpenGLESDispatch/gles1_only_functions.h"
#include "OpenGLESDispatch/gles1_extensions_functions.h"
#include "OpenGLESDispatch/gles2_only_functions.h"
#include "OpenGLESDispatch/gles2_extensions_functions.h"
#include "OpenGLESDispatch/gles3_only_functions.h"

// As a special case, LIST_GLES3_ONLY_FUNCTIONS below uses the Y parameter
// instead of the X one, meaning that the corresponding functions are
// optional extensions. This is only because currently, the only GLESv3
// API we support is glGetStringi(), which is not always provided by
// host desktop GL drivers (though most do).
#define LIST_GLES_FUNCTIONS(X,Y) \
    LIST_GLES_COMMON_FUNCTIONS(X) \
    LIST_GLES_EXTENSIONS_FUNCTIONS(Y) \
    LIST_GLES1_ONLY_FUNCTIONS(X) \
    LIST_GLES1_EXTENSIONS_FUNCTIONS(Y) \
    LIST_GLES2_ONLY_FUNCTIONS(X) \
    LIST_GLES2_EXTENSIONS_FUNCTIONS(Y) \
    LIST_GLES3_ONLY_FUNCTIONS(Y) \

#define LIST_GLES1_FUNCTIONS(X, Y) \
    LIST_GLES_COMMON_FUNCTIONS(X) \
    LIST_GLES_EXTENSIONS_FUNCTIONS(Y) \
    LIST_GLES1_ONLY_FUNCTIONS(X) \
    LIST_GLES1_EXTENSIONS_FUNCTIONS(Y) \

#define LIST_GLES2_FUNCTIONS(X, Y) \
    LIST_GLES_COMMON_FUNCTIONS(X) \
    LIST_GLES_EXTENSIONS_FUNCTIONS(Y) \
    LIST_GLES2_ONLY_FUNCTIONS(X) \
    LIST_GLES2_EXTENSIONS_FUNCTIONS(Y) \

