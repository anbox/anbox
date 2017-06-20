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

#include "anbox/application/database.h"
#include "anbox/wm/multi_window_manager.h"
#include "anbox/platform/base_platform.h"
#include "anbox/bridge/android_api_stub.h"
#include "anbox/logger.h"

#include <algorithm>

namespace anbox {
namespace wm {
MultiWindowManager::MultiWindowManager(const std::weak_ptr<platform::BasePlatform> &platform,
                                       const std::shared_ptr<bridge::AndroidApiStub> &android_api_stub,
                                       const std::shared_ptr<application::Database> &app_db)
    : platform_(platform), android_api_stub_(android_api_stub), app_db_(app_db) {}

MultiWindowManager::~MultiWindowManager() {}

void MultiWindowManager::apply_window_state_update(const WindowState::List &updated,
                                        const WindowState::List &removed) {
  std::lock_guard<std::mutex> l(mutex_);

  // Base on the update we get from the Android WindowManagerService we will
  // create different window instances with the properties supplied. Incoming
  // layer updates from SurfaceFlinger will be mapped later into those windows
  // and eventually composited there via GLES (e.g. for popups, ..)

  std::map<Task::Id, WindowState::List> task_updates;

  for (const auto &window : updated) {
    // Ignore all windows which are not part of the freeform task stack
    if (window.stack() != Stack::Id::Freeform) continue;

    // And also those which don't have a surface mapped at the moment
    if (!window.has_surface()) continue;

    // If we know that task already we first collect all window updates
    // for it so we can apply all of them together.
    auto w = windows_.find(window.task());
    if (w != windows_.end()) {
      auto t = task_updates.find(window.task());
      if (t == task_updates.end())
        task_updates.insert({window.task(), {window}});
      else
        task_updates[window.task()].push_back(window);
      continue;
    }

    auto title = window.package_name();
    auto app = app_db_->find_by_package(window.package_name());
    if (app.valid())
      title = app.name;

    if (auto p = platform_.lock()) {
      auto w = p->create_window(window.task(), window.frame(), title);
      if (w) {
        w->attach();
        windows_.insert({window.task(), w});
      } else {
        // FIXME can we call this here safely or do we need to schedule the removal?
        remove_task(window.task());
      }
    }
  }

  // Send updates we collected per task down to the corresponding window
  // so that they can update themself.
  for (const auto &u : task_updates) {
    auto w = windows_.find(u.first);
    if (w == windows_.end()) continue;

    w->second->update_state(u.second);
  }

  // As final step we process all windows we need to remove as they
  // got killed on the other side. We need to respect here that we
  // also get removals for windows which are part of a task which is
  // still in use by other windows.
  for (const auto &window : removed) {
    auto w = windows_.find(window.task());
    if (w == windows_.end()) continue;

    if (task_updates.find(window.task()) == task_updates.end()) {
      auto platform_window = w->second;
      platform_window->release();
      windows_.erase(w);
    }
  }
}

std::shared_ptr<Window> MultiWindowManager::find_window_for_task(const Task::Id &task) {
  std::lock_guard<std::mutex> l(mutex_);
  for (const auto &w : windows_) {
    if (w.second->task() == task) return w.second;
  }
  return nullptr;
}

void MultiWindowManager::resize_task(const Task::Id &task, const anbox::graphics::Rect &rect,
                                      const std::int32_t &resize_mode) {
  android_api_stub_->resize_task(task, rect, resize_mode);
}

void MultiWindowManager::set_focused_task(const Task::Id &task) {
  android_api_stub_->set_focused_task(task);
}

void MultiWindowManager::remove_task(const Task::Id &task) {
  android_api_stub_->remove_task(task);
}
}  // namespace wm
}  // namespace anbox
