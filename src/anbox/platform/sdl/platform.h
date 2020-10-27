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

#ifndef ANBOX_PLATFORM_SDL_PLATFORM_H_
#define ANBOX_PLATFORM_SDL_PLATFORM_H_

#include "anbox/platform/sdl/window.h"
#include "anbox/platform/sdl/sdl_wrapper.h"
#include "anbox/platform/base_platform.h"
#include "anbox/graphics/emugl/DisplayManager.h"
#include "anbox/input/device.h"

#include <map>
#include <thread>

class Renderer;

namespace anbox {
namespace input {
class Device;
class Manager;
}  // namespace input
namespace wm {
class Manager;
} // namespace wm
namespace platform {
namespace sdl {
class Platform : public std::enable_shared_from_this<Platform>,
                       public platform::BasePlatform,
                       public Window::Observer {
 public:
  Platform(const std::shared_ptr<input::Manager> &input_manager,
           const Configuration &config);
  ~Platform();

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

  void set_renderer(const std::shared_ptr<Renderer> &renderer) override;
  void set_window_manager(const std::shared_ptr<wm::Manager> &window_manager) override;

  void set_clipboard_data(const ClipboardData &data) override;
  ClipboardData get_clipboard_data() override;

  std::shared_ptr<audio::Sink> create_audio_sink(const std::shared_ptr<network::LocalSocketMessenger> &messenger) override;
  std::shared_ptr<audio::Source> create_audio_source() override;

  bool supports_multi_window() const override;

 private:
  void process_events();
  void process_input_event(const SDL_Event &event);

  bool adjust_coordinates(std::int32_t &x, std::int32_t &y);
  bool adjust_coordinates(SDL_Window *window, std::int32_t &x, std::int32_t &y);
  bool calculate_touch_coordinates(const SDL_Event &event, std::int32_t &x,
                                   std::int32_t &y);

  static Window::Id next_window_id();
  static constexpr std::uint32_t emulated_touch_id_ = 0;

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
  std::shared_ptr<input::Device> touch_;
  graphics::Rect display_frame_;
  bool window_size_immutable_ = false;
  std::uint32_t focused_sdl_window_id_ = 0;
  Configuration config_;

  static const int MAX_FINGERS = 10;
  static const int MAX_TRACKING_ID = 10;
  int touch_slots[MAX_FINGERS];
  int last_slot = -1;

  int find_touch_slot(int id);
  void push_slot(std::vector<input::Event> &touch_events, int slot);
  void push_finger_down(int x, int y, int finger_id, std::vector<input::Event> &touch_events);
  void push_finger_up(int finger_id, std::vector<input::Event> &touch_events);
  void push_finger_motion(int x, int y, int finger_id, std::vector<input::Event> &touch_events);
};
} // namespace sdl
} // namespace platform
} // namespace anbox

#endif
