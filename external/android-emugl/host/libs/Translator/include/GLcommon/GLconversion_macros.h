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
#ifndef _GL_FIXED_OPS_H
#define _GL_FIXED_OPS_H

#define X2F(x)        (((float)(x))/65536.0f)
#define X2D(x)        (((double)(x))/65536.0)
#define X2I(x)        ((x) /65536)
#define B2S(x)        ((short)x)


#define F2X(d) ((d) > 32767.65535 ? 32767 * 65536 + 65535 :  \
               (d) < -32768.65535 ? -32768 * 65536 + 65535 : \
               ((GLfixed) ((d) * 65536)))

#define I2X(d) ((d)*65536)

#endif
