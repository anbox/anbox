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

#pragma once

#include "anbox/wm/manager.h"

#include <map>
#include <memory>
#include <mutex>

namespace anbox::application {
  class Database;
}

namespace anbox::bridge {
  class AndroidApiStub;
}

namespace anbox::platform {
  class BasePlatform;
}

namespace anbox::wm {
class MultiWindowManager : public Manager {
 public:
  MultiWindowManager(const std::weak_ptr<platform::BasePlatform> &platform,
                     const std::shared_ptr<bridge::AndroidApiStub> &android_api_stub,
                     const std::shared_ptr<application::Database> &app_db);
  ~MultiWindowManager();

  void apply_window_state_update(const WindowState::List &updated, const WindowState::List &removed) override;

  std::shared_ptr<Window> find_window_for_task(const Task::Id &task) override;

  void resize_task(const Task::Id &task, const anbox::graphics::Rect &rect,
                   const std::int32_t &resize_mode) override;
  void set_focused_task(const Task::Id &task) override;
  void remove_task(const Task::Id &task) override;

 private:
  std::mutex mutex_;
  std::weak_ptr<platform::BasePlatform> platform_;
  std::shared_ptr<bridge::AndroidApiStub> android_api_stub_;
  std::shared_ptr<application::Database> app_db_;
  std::map<Task::Id, std::shared_ptr<Window>> windows_;
};
}
