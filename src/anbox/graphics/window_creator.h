/*
 * Copyright (C) 2016 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ANBOX_GRAPHICS_WINDOW_CREATOR_H_
#define ANBOX_GRAPHICS_WINDOW_CREATOR_H_

#include "external/android-emugl/host/libs/libOpenglRender/NativeSubWindow.h"
#include "external/android-emugl/host/libs/libOpenglRender/DisplayManager.h"

namespace anbox {
namespace input {
class Manager;
} // namespace input
namespace graphics {
class WindowCreator : public SubWindowHandler,
                      public DisplayManager {
public:
    WindowCreator(const std::shared_ptr<input::Manager> &input_manager);
    virtual ~WindowCreator();

    virtual EGLNativeDisplayType native_display() const = 0;

protected:
    std::shared_ptr<input::Manager> input_manager_;
};
} // namespace graphics
} // namespace anbox
#endif
