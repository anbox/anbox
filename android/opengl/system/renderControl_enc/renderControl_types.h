/*
* Copyright 2011 The Android Open Source Project
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

#include <stdint.h>
#include <EGL/egl.h>
#include "glUtils.h"

// values for 'param' argument of rcGetFBParam
#define FB_WIDTH    1
#define FB_HEIGHT   2
#define FB_XDPI     3
#define FB_YDPI     4
#define FB_FPS      5
#define FB_MIN_SWAP_INTERVAL 6
#define FB_MAX_SWAP_INTERVAL 7
