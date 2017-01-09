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
#ifndef _ERROR_LOG_H_
#define _ERROR_LOG_H_

#if defined(__ANDROID__)
#    include <cutils/log.h>
#    define ERR(...)    ALOGE(__VA_ARGS__)
#    ifdef EMUGL_DEBUG
#        define DBG(...)    ALOGD(__VA_ARGS__)
#    else
#        define DBG(...)    ((void)0)
#    endif
#else
#     include <stdio.h>
#    define ERR(...)    fprintf(stderr, __VA_ARGS__)
#    ifdef EMUGL_DEBUG
#        define DBG(...)    fprintf(stderr, __VA_ARGS__)
#    else
#        define DBG(...)    ((void)0)
#    endif
#endif

#endif
