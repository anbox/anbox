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

#ifndef ANBOX_GRAPHICS_MIR_NATIVE_WINDOW_CREATOR_H_
#define ANBOX_GRAPHICS_MIR_NATIVE_WINDOW_CREATOR_H_

#include "external/android-emugl/host/libs/libOpenglRender/NativeSubWindow.h"

#include <memory>
#include <map>

namespace anbox {
namespace input {
class Manager;
}
namespace graphics {

class MirDisplayConnection;
class MirWindow;

class MirNativeWindowCreator : public SubWindowHandler {
public:
    MirNativeWindowCreator(const std::shared_ptr<input::Manager> &input_channel);
    virtual ~MirNativeWindowCreator();

    EGLNativeWindowType create_window(int x, int y, int width, int height) override;
    void destroy_window(EGLNativeWindowType win) override;

    std::shared_ptr<MirDisplayConnection> display() const;

private:
    std::shared_ptr<input::Manager> input_manager_;
    std::shared_ptr<MirDisplayConnection> display_connection_;
    std::map<EGLNativeWindowType,std::shared_ptr<MirWindow>> windows_;
};

} // namespace graphics
} // namespace anbox

#endif
