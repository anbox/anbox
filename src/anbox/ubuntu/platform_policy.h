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

#ifndef ANBOX_UBUNTU_PLATFORM_POLICY_H_
#define ANBOX_UBUNTU_PLATFORM_POLICY_H_

#include "anbox/wm/platform_policy.h"
#include "anbox/ubuntu/window.h"

#include "anbox/graphics/emugl/DisplayManager.h"

#include <thread>
#include <map>

#include <SDL.h>

namespace anbox {
namespace input {
class Device;
class Manager;
} // namespace input
namespace bridge {
class AndroidApiStub;
} // namespace bridge
namespace ubuntu {
class PlatformPolicy : public std::enable_shared_from_this<PlatformPolicy>,
                       public wm::PlatformPolicy,
                       public Window::Observer,
                       public DisplayManager {
public:
    PlatformPolicy(const std::shared_ptr<input::Manager> &input_manager,
                   const std::shared_ptr<bridge::AndroidApiStub> &android_api);
    ~PlatformPolicy();

    std::shared_ptr<wm::Window> create_window(const anbox::wm::Task::Id &task, const anbox::graphics::Rect &frame) override;

    void window_deleted(const Window::Id &id) override;
    void window_wants_focus(const Window::Id &id) override;

    DisplayInfo display_info() const override;

private:
    void process_events();
    void process_input_event(const SDL_Event &event);

    static Window::Id next_window_id();

    std::shared_ptr<input::Manager> input_manager_;
    std::shared_ptr<bridge::AndroidApiStub> android_api_;
    // We don't own the windows anymore after the got created by us so we
    // need to be careful once we try to use them again.
    std::map<Window::Id, std::weak_ptr<Window>> windows_;
    std::shared_ptr<Window> current_window_;
    std::thread event_thread_;
    bool event_thread_running_;
    std::shared_ptr<input::Device> pointer_;
    std::shared_ptr<input::Device> keyboard_;
    DisplayManager::DisplayInfo display_info_;
};
} // namespace wm
} // namespace anbox

#endif
