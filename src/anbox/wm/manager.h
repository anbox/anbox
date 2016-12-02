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

#ifndef ANBOX_WM_MANAGER_H_
#define ANBOX_WM_MANAGER_H_

#include "anbox/wm/window.h"
#include "anbox/wm/window_state.h"

#include <map>
#include <memory>
#include <mutex>

namespace anbox {
namespace wm {
class PlatformPolicy;
class Manager {
 public:
  Manager(const std::shared_ptr<PlatformPolicy> &platform);
  ~Manager();

  void apply_window_state_update(const WindowState::List &updated,
                                 const WindowState::List &removed);

  std::shared_ptr<Window> find_window_for_task(const Task::Id &task);

 private:
  std::mutex mutex_;
  std::shared_ptr<PlatformPolicy> platform_;
  std::map<Task::Id, std::shared_ptr<Window>> windows_;
};
}  // namespace wm
}  // namespace anbox

#endif
