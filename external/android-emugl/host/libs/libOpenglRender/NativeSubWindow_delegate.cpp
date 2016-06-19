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

#include <stdexcept>
#include <string>
#include <map>

namespace {
static std::shared_ptr<SubWindowHandler> current_handler = nullptr;
}

void registerSubWindowHandler(const std::shared_ptr<SubWindowHandler> &handler) {
    if (current_handler)
        throw std::runtime_error("A sub window handle is already registered");

    current_handler = handler;
}

EGLNativeWindowType createSubWindow(FBNativeWindowType p_window,
                                    int x,
                                    int y,
                                    int width,
                                    int height,
                                    SubWindowRepaintCallback repaint_callback,
                                    void* repaint_callback_param) {
    (void) p_window;
    (void) repaint_callback;
    (void) repaint_callback_param;

    if (!current_handler)
        return (EGLNativeWindowType) 0;

    return current_handler->create_window(x, y, width, height);
}

void destroySubWindow(EGLNativeWindowType win) {
    if (!current_handler)
        return;

    return current_handler->destroy_window(win);
}

int moveSubWindow(FBNativeWindowType p_parent_window,
                  EGLNativeWindowType p_sub_window,
                  int x,
                  int y,
                  int width,
                  int height) {
    (void) p_parent_window;
    (void) p_sub_window;
    (void) x;
    (void) y;
    (void) width;
    (void) height;

    return true;
}
