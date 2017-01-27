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

#include "anbox/ubuntu/window.h"
#include "anbox/platform/policy.h"

#include "anbox/graphics/emugl/DisplayManager.h"

#include <map>
#include <thread>

#include <SDL.h>

class Renderer;

namespace anbox {
namespace input {
class Device;
class Manager;
}  // namespace input
namespace wm {
class Manager;
} // namespace wm
namespace ubuntu {
class PlatformPolicy : public std::enable_shared_from_this<PlatformPolicy>,
                       public platform::Policy,
                       public Window::Observer,
                       public DisplayManager {
 public:
  PlatformPolicy(const std::shared_ptr<input::Manager> &input_manager,
                 const graphics::Rect &static_display_frame = graphics::Rect::Invalid,
                 bool single_window = false);
  ~PlatformPolicy();

  std::shared_ptr<wm::Window> create_window(
      const anbox::wm::Task::Id &task,
      const anbox::graphics::Rect &frame,
      const std::string &title) override;

  void window_deleted(const Window::Id &id) override;
  void window_wants_focus(const Window::Id &id) override;
  void window_moved(const Window::Id &id, const std::int32_t &x,
                    const std::int32_t &y) override;
  void window_resized(const Window::Id &id, const std::int32_t &width,
                      const std::int32_t &height) override;

  DisplayInfo display_info() const override;

  void set_renderer(const std::shared_ptr<Renderer> &renderer);
  void set_window_manager(const std::shared_ptr<wm::Manager> &window_manager);

  void set_clipboard_data(const ClipboardData &data) override;
  ClipboardData get_clipboard_data() override;

  std::shared_ptr<audio::Sink> create_audio_sink() override;
  std::shared_ptr<audio::Source> create_audio_source() override;

 private:
  void process_events();
  void process_input_event(const SDL_Event &event);

  static Window::Id next_window_id();

  std::shared_ptr<Renderer> renderer_;
  std::shared_ptr<input::Manager> input_manager_;
  std::shared_ptr<wm::Manager> window_manager_;
  // We don't own the windows anymore after the got created by us so we
  // need to be careful once we try to use them again.
  std::map<Window::Id, std::weak_ptr<Window>> windows_;
  std::shared_ptr<Window> current_window_;
  std::thread event_thread_;
  bool event_thread_running_;
  std::shared_ptr<input::Device> pointer_;
  std::shared_ptr<input::Device> keyboard_;
  DisplayManager::DisplayInfo display_info_;
  bool window_size_immutable_ = false;
  bool single_window_ = false;
};
}  // namespace wm
}  // namespace anbox

#endif
