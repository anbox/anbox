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

#ifndef ANBOX_PLATFORM_NULL_PLATFORM_H_
#define ANBOX_PLATFORM_NULL_PLATFORM_H_

#include "anbox/platform/base_platform.h"

namespace anbox {
namespace platform {
class NullPlatform : public BasePlatform {
 public:
  NullPlatform();
  std::shared_ptr<wm::Window> create_window(
      const anbox::wm::Task::Id &task,
      const anbox::graphics::Rect &frame,
      const std::string &title) override;
  void set_clipboard_data(const ClipboardData &data) override;
  ClipboardData get_clipboard_data() override;
  std::shared_ptr<audio::Sink> create_audio_sink() override;
  std::shared_ptr<audio::Source> create_audio_source() override;
  void set_renderer(const std::shared_ptr<Renderer> &renderer) override;
  void set_window_manager(const std::shared_ptr<wm::Manager> &window_manager) override;
  bool supports_multi_window() const override;
};
}  // namespace wm
}  // namespace anbox

#endif
