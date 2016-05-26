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
#include "NativeSubWindow.h"
#include <Cocoa/Cocoa.h>

/*
 * EmuGLView inherit from NSView and override the isOpaque
 * method to return YES. That prevents drawing of underlying window/view
 * when the view needs to be redrawn.
 */
@interface EmuGLView : NSView {
} @end

@implementation EmuGLView

  - (BOOL)isOpaque {
      return YES;
  }

@end


EGLNativeWindowType createSubWindow(FBNativeWindowType p_window,
                                    int x,
                                    int y,
                                    int width,
                                    int height,
                                    SubWindowRepaintCallback repaint_callback,
                                    void* repaint_callback_param) {
    NSWindow* win = (NSWindow *)p_window;
    if (!win) {
        return NULL;
    }

    /* (x,y) assume an upper-left origin, but Cocoa uses a lower-left origin */
    NSRect content_rect = [win contentRectForFrameRect:[win frame]];
    int cocoa_y = (int)content_rect.size.height - (y + height);
    NSRect contentRect = NSMakeRect(x, cocoa_y, width, height);

    NSView *glView = [[EmuGLView alloc] initWithFrame:contentRect];
    if (glView) {
        [glView setWantsBestResolutionOpenGLSurface:YES];
        [[win contentView] addSubview:glView];
        [win makeKeyAndOrderFront:nil];
    }

    return (EGLNativeWindowType)glView;
}

void destroySubWindow(EGLNativeWindowType win) {
    if(win){
        NSView *glView = (NSView *)win;
        [glView removeFromSuperview];
        [glView release];
    }
}

int moveSubWindow(FBNativeWindowType p_parent_window,
                  EGLNativeWindowType p_sub_window,
                  int x,
                  int y,
                  int width,
                  int height) {
    NSWindow *win = (NSWindow *)p_parent_window;
    if (!win) {
        return 0;
    }

    NSView *glView = (NSView *)p_sub_window;
    if (!glView) {
        return 0;
    }

    /* The view must be removed from the hierarchy to be properly resized */
    [glView removeFromSuperview];

    /* (x,y) assume an upper-left origin, but Cocoa uses a lower-left origin */
    NSRect content_rect = [win contentRectForFrameRect:[win frame]];
    int cocoa_y = (int)content_rect.size.height - (y + height);
    NSRect newFrame = NSMakeRect(x, cocoa_y, width, height);
    [glView setFrame:newFrame];

    /* Re-add the sub-window to the view hierarchy */
    [[win contentView] addSubview:glView];
    [win makeKeyAndOrderFront:nil];

    return 1;
}
