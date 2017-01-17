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

#include "anbox/platform/default_policy.h"
#include "anbox/wm/window.h"
#include "anbox/logger.h"

namespace {
class NullWindow : public anbox::wm::Window {
 public:
  NullWindow(const anbox::wm::Task::Id &task,
             const anbox::graphics::Rect &frame,
             const std::string &title)
      : anbox::wm::Window(nullptr, task, frame, title) {}
};
}

namespace anbox {
namespace platform {
DefaultPolicy::DefaultPolicy() {}

std::shared_ptr<wm::Window> DefaultPolicy::create_window(
    const anbox::wm::Task::Id &task, const anbox::graphics::Rect &frame, const std::string &title) {
  return std::make_shared<::NullWindow>(task, frame, title);
}

void DefaultPolicy::set_clipboard_data(const ClipboardData &data) {
  (void)data;
  ERROR("Not implemented");
}

DefaultPolicy::ClipboardData DefaultPolicy::get_clipboard_data() {
  ERROR("Not implemented");
  return ClipboardData{};
}

std::shared_ptr<audio::Sink> DefaultPolicy::create_audio_sink() {
  ERROR("Not implemented");
  return nullptr;
}

std::shared_ptr<audio::Source> DefaultPolicy::create_audio_source() {
  ERROR("Not implemented");
  return nullptr;
}
}  // namespace wm
}  // namespace anbox
