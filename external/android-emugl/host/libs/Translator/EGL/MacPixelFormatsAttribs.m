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

#include "MacPixelFormatsAttribs.h"

static NSOpenGLPixelFormatAttribute attrs32_1[] =
{
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAWindow,
    NSOpenGLPFAPixelBuffer,
    NSOpenGLPFAColorSize   ,32,
    NSOpenGLPFADepthSize   ,24,
    NSOpenGLPFAStencilSize ,8,
    0
};

static NSOpenGLPixelFormatAttribute attrs32_2[] =
{
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAWindow,
    NSOpenGLPFAPixelBuffer,
    NSOpenGLPFAColorSize   ,32,
    NSOpenGLPFAAlphaSize   ,8,
    NSOpenGLPFADepthSize   ,24,
    NSOpenGLPFAStencilSize ,8,
    0
};

static NSOpenGLPixelFormatAttribute attrs32_3[] =
{
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAWindow,
    NSOpenGLPFAPixelBuffer,
    NSOpenGLPFAColorSize   ,32,
    NSOpenGLPFAAlphaSize   ,8,
    0
};

static NSOpenGLPixelFormatAttribute attrs32_4[] =
{
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAWindow,
    NSOpenGLPFAPixelBuffer,
    NSOpenGLPFAColorSize   ,32,
    0
};

static NSOpenGLPixelFormatAttribute attrs32_5[] =
{
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAWindow,
    NSOpenGLPFAPixelBuffer,
    NSOpenGLPFAColorSize   ,32,
    NSOpenGLPFADepthSize   ,24,
    NSOpenGLPFASamples     ,2,
    0
};

static NSOpenGLPixelFormatAttribute attrs32_6[] =
{
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAWindow,
    NSOpenGLPFAPixelBuffer,
    NSOpenGLPFAColorSize   ,32,
    NSOpenGLPFADepthSize   ,24,
    NSOpenGLPFASamples     ,4,
    0
};

static NSOpenGLPixelFormatAttribute attrs32_7[] =
{
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAWindow,
    NSOpenGLPFAPixelBuffer,
    NSOpenGLPFAColorSize   ,32,
    NSOpenGLPFAAlphaSize   ,8,
    NSOpenGLPFADepthSize   ,24,
    NSOpenGLPFAStencilSize ,8,
    NSOpenGLPFASamples     ,4,
    0
};

static NSOpenGLPixelFormatAttribute attrs16_1[] =
{
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAWindow,
    NSOpenGLPFAPixelBuffer,
    NSOpenGLPFAColorSize   ,16,
    NSOpenGLPFADepthSize   ,24,
    0
};

static NSOpenGLPixelFormatAttribute attrs16_2[] =
{
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAWindow,
    NSOpenGLPFAPixelBuffer,
    NSOpenGLPFAColorSize   ,16,
    NSOpenGLPFADepthSize   ,24,
    NSOpenGLPFAStencilSize ,8,
    0
};

static NSOpenGLPixelFormatAttribute attrs64_1[] =
{
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAWindow,
    NSOpenGLPFAPixelBuffer,
    NSOpenGLPFAColorSize   ,64,
    NSOpenGLPFAAlphaSize   ,16,
    0
};

static NSOpenGLPixelFormatAttribute attrs64_2[] =
{
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAWindow,
    NSOpenGLPFAPixelBuffer,
    NSOpenGLPFAColorSize   ,64,
    NSOpenGLPFAAlphaSize   ,16,
    NSOpenGLPFADepthSize   ,24,
    NSOpenGLPFAStencilSize ,8,
    0
};

static NSOpenGLPixelFormatAttribute attrs64_3[] =
{
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAWindow,
    NSOpenGLPFAPixelBuffer,
    NSOpenGLPFAColorSize   ,64,
    NSOpenGLPFAAlphaSize   ,16,
    NSOpenGLPFADepthSize   ,24,
    0
};

static NSOpenGLPixelFormatAttribute attrs64_4[] =
{
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAWindow,
    NSOpenGLPFAPixelBuffer,
    NSOpenGLPFAColorSize   ,64,
    NSOpenGLPFADepthSize   ,24,
    0
};

static NSOpenGLPixelFormatAttribute attrs64_5[] =
{
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAWindow,
    NSOpenGLPFAPixelBuffer,
    NSOpenGLPFAColorSize   ,64,
    NSOpenGLPFADepthSize   ,24,
    NSOpenGLPFAStencilSize ,8,
    0
};

static NSOpenGLPixelFormatAttribute attrs128_1[] =
{
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAWindow,
    NSOpenGLPFAPixelBuffer,
    NSOpenGLPFAColorSize   ,128,
    NSOpenGLPFAAlphaSize   ,32,
    0
};

static NSOpenGLPixelFormatAttribute attrs128_2[] =
{
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAWindow,
    NSOpenGLPFAPixelBuffer,
    NSOpenGLPFAColorSize   ,128,
    NSOpenGLPFAAlphaSize   ,32,
    NSOpenGLPFADepthSize   ,24,
    0
};

NSOpenGLPixelFormatAttribute** getPixelFormatsAttributes(int* size){
static NSOpenGLPixelFormatAttribute* arr[] =
{
    attrs16_1,
    attrs16_2,
    attrs32_1,
    attrs32_2,
    attrs32_3,
    attrs32_4,
    attrs32_5,
    attrs32_6,
    attrs32_7,
    attrs64_1,
    attrs64_2,
    attrs64_3,
    attrs64_4,
    attrs64_5,
    attrs128_1,
    attrs128_2
};
    *size = sizeof(arr)/sizeof(arr[0]);
    return arr;
}
