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

#ifndef ANBOX_PLATFORM_POLICY_H_
#define ANBOX_PLATFORM_POLICY_H_

#include "anbox/graphics/rect.h"
#include "anbox/wm/window_state.h"
#include "anbox/network/local_socket_messenger.h"

#include <memory>

class Renderer;

namespace anbox {
namespace audio {
class Sink;
class Source;
} // namespace audio
namespace wm {
class Window;
class Manager;
} // namespace wm
namespace input {
class Manager;
} // namespace input
namespace platform {
class BasePlatform {
 public:
  virtual ~BasePlatform() {}

  virtual std::shared_ptr<wm::Window> create_window(const anbox::wm::Task::Id &task, const anbox::graphics::Rect &frame, const std::string &title) = 0;

  struct ClipboardData {
    std::string text;
  };

  virtual void set_clipboard_data(const ClipboardData &data) = 0;
  virtual ClipboardData get_clipboard_data() = 0;

  virtual std::shared_ptr<audio::Sink> create_audio_sink(const std::shared_ptr<anbox::network::LocalSocketMessenger> &messenger) = 0;
  virtual std::shared_ptr<audio::Source> create_audio_source() = 0;

  virtual void set_renderer(const std::shared_ptr<Renderer> &renderer) = 0;
  virtual void set_window_manager(const std::shared_ptr<wm::Manager> &window_manager) = 0;

  virtual bool supports_multi_window() const = 0;
};

struct Configuration {
  graphics::Rect display_frame = graphics::Rect::Invalid;
  bool single_window = false;
  bool no_touch_emulation = false;
};

std::shared_ptr<BasePlatform> create(const std::string &name,
                                     const std::shared_ptr<input::Manager> &input_manager,
                                     const Configuration &config);
}  // namespace platform
}  // namespace anbox

#endif
