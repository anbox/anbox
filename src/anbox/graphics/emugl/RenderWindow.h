// Copyright 2014-2015 The Android Open Source Project
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

#ifndef ANDROID_EMUGL_LIBRENDER_RENDER_WINDOW_H
#define ANDROID_EMUGL_LIBRENDER_RENDER_WINDOW_H

#include "OpenglRender/render_api.h"

namespace emugl {
class Thread;
}  // namespace emugl

class RenderWindowChannel;
struct RenderWindowMessage;

// Helper class used to manage the sub-window that displays the emulated GPU
// output. To use it, do the following:
//
//  1) Create a new instance, passing the size of the emulated accelerated
//     framebuffer in pixels you need.
//
//  2) Check isValid() after construction. If false, the library could not
//     initialize the class properly, and one should abort.
//
//  3) Optional: call setPostCallback() to specify a callback function which
//     will be called everytime a new frame is drawn.
//
//  4) Call setupSubWindow() to setup a new sub-window within the UI window.
//     One can call removeSubWindow() to remove it, and one can call
//     setupSubWindow() + removeSubWindow() any number of time (e.g. for
//     changing the position / rotation of the subwindow).
//
//  5) Optional: call setRotation() to only change the display rotation of
//     the sub-window content.
//
//  6) Call repaint() to force a repaint().
//
class RenderWindow {
 public:
  // Create new instance. |width| and |height| are the dimensions of the
  // emulated accelerated framebuffer. |use_thread| can be true to force
  // the use of a separate thread, which might be required on some platforms
  // to avoid GL-realted corruption issues in the main window. Call
  // isValid() after construction to verify that it worked properly.
  //
  // |use_sub_window| is true if the client will call setupSubWindow(),
  // and false if it will call setPostCallback().
  //
  // Note that this call doesn't display anything, it just initializes
  // the library, use setupSubWindow() to display something.
  RenderWindow(EGLNativeDisplayType native_display, bool use_thread);

  // Destructor. This will automatically call removeSubWindow() is needed.
  ~RenderWindow();

  // Returns true if the RenderWindow instance is valid, which really
  // means that the constructor succeeded.
  bool isValid() const { return mValid; }

  // Return misc. GL strings to the caller. On success, return true and sets
  // |*vendor| to the GL vendor string, |*renderer| to the GL renderer one,
  // and |*version| to the GL version one. On failure, return false.
  bool getHardwareStrings(const char** vendor, const char** renderer,
                          const char** version);

  // Start displaying the emulated framebuffer using a sub-window of a
  // parent |window| id. |wx|, |wy|, |ww| and |wh| are the position
  // and dimension of the sub-window, relative to its parent.
  // |fbw| and |fbh| are the dimensions of the underlying guest framebuffer.
  // |dpr| is the device pixel ratio for the monitor, which is required for
  // higher-density displays (such as retina).
  // |rotation| is a clockwise-rotation for the content. Only multiples of
  // 90. are accepted. Returns true on success, false otherwise.
  //
  // If the subwindow already exists, this function will update
  // the dimensions of the subwindow, backing framebuffer, and rendering
  // pipeline to reflect the new values.
  //
  // One can call removeSubWindow() to remove the sub-window.
  bool setupSubWindow(FBNativeWindowType window, int wx, int wy, int ww, int wh,
                      int fbw, int fbh, float dpr, float rotation);

  // Remove the sub-window created by calling setupSubWindow().
  // Note that this doesn't discard the content of the emulated framebuffer,
  // it just hides it from the main window. Returns true on success, false
  // otherwise.
  bool removeSubWindow();

  // Change the display rotation on the fly. |zRot| is a clockwise rotation
  // angle in degrees. Only multiples of 90. are accepted.
  void setRotation(float zRot);

  // Change the display translation. |px|,|py| are numbers between 0 and 1,
  // with (0,0) indicating "align the bottom left of the framebuffer with the
  // bottom left of the subwindow", and (1,1) indicating "align the top right of
  // the framebuffer with the top right of the subwindow."
  void setTranslation(float px, float py);

  // Force a repaint of the whole content into the sub-window.
  void repaint();

 private:
  bool processMessage(const RenderWindowMessage& msg);

  bool mValid;
  bool mHasSubWindow;
  emugl::Thread* mThread;
  RenderWindowChannel* mChannel;
};

#endif  // ANDROID_EMUGL_LIBRENDER_RENDER_WINDOW_H
