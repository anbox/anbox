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

#ifndef ANBOX_UBUNTU_WINDOW_H_
#define ANBOX_UBUNTU_WINDOW_H_

#include "anbox/wm/window.h"

#include <EGL/egl.h>

#include <memory>
#include <vector>

#include <SDL.h>

namespace anbox {
namespace ubuntu {
class Window : public std::enable_shared_from_this<Window>,
               public wm::Window {
public:
    typedef std::int32_t Id;
    static Id Invalid;

    class Observer {
    public:
        virtual ~Observer();
        virtual void window_deleted(const Id &id) = 0;
    };

    Window(const Id &id, const wm::Task::Id &task, const std::shared_ptr<Observer> &observer, const graphics::Rect &frame);
    Window(int x, int y, int width, int height);
    ~Window();

    void process_event(const SDL_Event &event);

    EGLNativeWindowType native_handle() const override;
    Id id() const;
    std::uint32_t window_id() const;

protected:
    void resize(int width, int height) override;
    void update_position(int x, int y) override;

private:
    Id id_;
    std::shared_ptr<Observer> observer_;
    EGLNativeDisplayType native_display_;
    EGLNativeWindowType native_window_;
    SDL_Window *window_;
};
} // namespace bridge
} // namespace anbox

#endif
