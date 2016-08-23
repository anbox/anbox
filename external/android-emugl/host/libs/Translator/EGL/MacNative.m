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
#include <stdio.h>
#include <Cocoa/Cocoa.h>
#include <OpenGL/OpenGL.h>
#include "MacPixelFormatsAttribs.h"

//
// EmuGLContext inherit from NSOpenGLContext
// and adds binding state for the context to know
// if it was last bounded to a pbuffer or a window.
// This is because after the context was bounded to
// a Pbuffer, before we bind it to a window we must
// release it form the pbuffer by calling the
// clearDrawable method. We do not want to call clearDrawable
// more than really needed since when it is called at a time
// that a window is bounded to the context it will clear the
// window content causing flickering effect.
// Thererfore we call clearDrawable only when we bind the context
// to a window and it was previously bound to a Pbuffer.
//
@interface EmuGLContext : NSOpenGLContext {
    @private
        int boundToPbuffer;
        int boundToWin;
}

- (id) initWithFormat:(NSOpenGLPixelFormat *)pixelFormat shareContext:(NSOpenGLContext *)share;
- (void) preBind:(int)forPbuffer;
@end

@implementation EmuGLContext
- (id) initWithFormat:(NSOpenGLPixelFormat *)pixelFormat shareContext:(NSOpenGLContext *)share
{
    self = [super initWithFormat:pixelFormat shareContext:share];
    if (self != nil) {
        boundToPbuffer = 0;
        boundToWin = 0;
    }
    return self;
}

- (void) preBind:(int)forPbuffer
{
    if ((!forPbuffer && boundToPbuffer)) {
        [self clearDrawable]; 
    }
    boundToPbuffer = forPbuffer;
    boundToWin = !boundToPbuffer;
}
@end

int getNumPixelFormats(){
    int size;
    NSOpenGLPixelFormatAttribute** attrib_lists = getPixelFormatsAttributes(&size);
    return size;
}

void* getPixelFormat(int i){
    int size;
    NSOpenGLPixelFormatAttribute** attrib_lists = getPixelFormatsAttributes(&size);
    return [[NSOpenGLPixelFormat alloc] initWithAttributes:attrib_lists[i]];
}

int getPixelFormatDefinitionAlpha(int i) {
    int size;
    NSOpenGLPixelFormatAttribute** attrib_lists = getPixelFormatsAttributes(&size);
    NSOpenGLPixelFormatAttribute* attribs = attrib_lists[i];
    while (*attribs) {
        switch (*attribs) {
        // These are the ones that take a value, according to the current
        // NSOpenGLPixelFormat docs
        case NSOpenGLPFAAuxBuffers:
        case NSOpenGLPFAColorSize:
        case NSOpenGLPFADepthSize:
        case NSOpenGLPFAStencilSize:
        case NSOpenGLPFAAccumSize:
        case NSOpenGLPFARendererID:
        case NSOpenGLPFAScreenMask:
            attribs += 2;
            break;
        case NSOpenGLPFAAlphaSize:
            return attribs[1];
            break;
        // All other attributes are boolean attributes that don't take a value
        default:
            attribs++;
        }
    }
    return 0;
}

void getPixelFormatAttrib(void* pixelFormat,int attrib,int* val){
    NSOpenGLPixelFormat *frmt = (NSOpenGLPixelFormat *)pixelFormat;
    [frmt getValues:val forAttribute:attrib forVirtualScreen:0];
}

void* nsCreateContext(void* format,void* share){
    NSOpenGLPixelFormat* frmt = (NSOpenGLPixelFormat*)format;
    return [[EmuGLContext alloc] initWithFormat:frmt shareContext:share];
}

void  nsPBufferMakeCurrent(void* context,void* nativePBuffer,int level){
    EmuGLContext* ctx = (EmuGLContext *)context;
    NSOpenGLPixelBuffer* pbuff = (NSOpenGLPixelBuffer *)nativePBuffer;
    if(ctx == nil){
        [NSOpenGLContext clearCurrentContext];
    } else {
        if(pbuff != nil){
            [ctx preBind:1];
            [ctx setPixelBuffer:pbuff cubeMapFace:0 mipMapLevel:level currentVirtualScreen:0];
            [ctx makeCurrentContext];
        }
    }
}

void nsWindowMakeCurrent(void* context,void* nativeWin){
    EmuGLContext* ctx = (EmuGLContext *)context;
    NSView* win = (NSView *)nativeWin;
    if(ctx == nil){
        [NSOpenGLContext clearCurrentContext];
    } else if (win != nil) {
        [ctx preBind:0];
        [ctx setView: win];
        [ctx makeCurrentContext];
    }
}

void nsSwapBuffers(){
    NSOpenGLContext* ctx = [NSOpenGLContext currentContext];
    if(ctx != nil){
        [ctx flushBuffer];
    }
}

void nsSwapInterval(int *interval){
    NSOpenGLContext* ctx = [NSOpenGLContext currentContext];
    if( ctx != nil){
        [ctx setValues:interval forParameter:NSOpenGLCPSwapInterval];
    }
}


void nsDestroyContext(void* context){
    EmuGLContext *ctx = (EmuGLContext*)context;
    if(ctx != nil){
        [ctx release];
    }
}


void* nsCreatePBuffer(GLenum target,GLenum format,int maxMip,int width,int height){
    return [[NSOpenGLPixelBuffer alloc] initWithTextureTarget:target
                                        textureInternalFormat:format
                                        textureMaxMipMapLevel:maxMip
                                        pixelsWide:width pixelsHigh:height];

}

void nsDestroyPBuffer(void* pbuffer){
    NSOpenGLPixelBuffer *pbuf = (NSOpenGLPixelBuffer*)pbuffer;
    if(pbuf != nil){
        [pbuf release];
    }
}

bool nsGetWinDims(void* win,unsigned int* width,unsigned int* height){
    NSView* view = (NSView*)win;
    if(view != nil){
        NSRect rect = [view bounds];
        *width  = rect.size.width;
        *height = rect.size.height;
        return true;
    }
    return false;
}

bool  nsCheckColor(void* win,int colorSize){
    NSView* view = (NSView*)win;
   if(view != nil){
       NSWindow* wnd = [view window];
       if(wnd != nil){
           NSWindowDepth limit = [wnd depthLimit];
           NSWindowDepth defaultLimit = [NSWindow defaultDepthLimit];

           int depth = (limit != 0) ? NSBitsPerPixelFromDepth(limit):
                                      NSBitsPerPixelFromDepth(defaultLimit);
           return depth >= colorSize;

       }
   }
   return false;

}
