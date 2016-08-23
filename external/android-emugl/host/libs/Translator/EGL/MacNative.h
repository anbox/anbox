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

#ifndef  MAC_NATIVE_H
#define  MAC_NATIVE_H

#include <OpenGL/gl.h>

typedef enum {                               // Mac equivalence
                 MAC_HAS_DOUBLE_BUFFER = 5,  // NSOpenGLPFADoubleBuffer
                 MAC_DRAW_TO_WINDOW    = 80, // NSOpenGLPFAWindow
                 MAC_DRAW_TO_PBUFFER   = 90, // NSOpenGLPFAPixelBuffer
                 MAC_SAMPLES_PER_PIXEL = 56, // NSOpenGLPFASamples
                 MAC_COLOR_SIZE        = 8,  // NSOpenGLPFAColorSize
                 MAC_ALPHA_SIZE        = 11, // NSOpenGLPFAAlphaSize
                 MAC_DEPTH_SIZE        = 12, // NSOpenGLPFADepthSize
                 MAC_STENCIL_SIZE      = 13  // NSOpenGLPFAStencilSize
             } MacPixelFormatAttribs;


extern "C"{

int   getNumPixelFormats();
void* getPixelFormat(int i);
int   getPixelFormatDefinitionAlpha(int i);
void  getPixelFormatAttrib(void* pixelFormat,int attrib,int* val);
void* nsCreateContext(void* format,void* share);
void  nsWindowMakeCurrent(void* context,void* nativeWin);
void  nsPBufferMakeCurrent(void* context,void* nativePBuffer,int level);
void  nsSwapBuffers();
void  nsSwapInterval(int *interval);
void  nsDestroyContext(void* context);
void* nsCreatePBuffer(GLenum target,GLenum format,int maxMip,int width,int height);
void  nsDestroyPBuffer(void* pbuffer);
bool  nsGetWinDims(void* win,unsigned int* width,unsigned int* height);
bool  nsCheckColor(void* win,int colorSize);

}

#endif
