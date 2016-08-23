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
#ifndef _CLIENT_APIS_EXTS_H
#define _CLIENT_APIS_EXTS_H

#include "GLcommon/TranslatorIfaces.h"

namespace ClientAPIExts
{

void initClientFuncs(const GLESiface *iface, int idx);
__translatorMustCastToProperFunctionPointerType getProcAddress(const char *fname);

} // of namespace ClientAPIExts

#endif
