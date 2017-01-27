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

#include "anbox/wm/single_window_manager.h"
#include "anbox/platform/policy.h"
#include "anbox/logger.h"

#include <algorithm>

#include <sys/types.h>
#include <signal.h>

namespace anbox {
namespace wm {
SingleWindowManager::SingleWindowManager(const std::shared_ptr<platform::Policy> &policy,
                                         const std::shared_ptr<application::Database> &app_db)
    : platform_policy_(policy), app_db_(app_db) {}

SingleWindowManager::~SingleWindowManager() {}

void SingleWindowManager::setup() {
  window_ = platform_policy_->create_window(0, {0, 0, 1024, 768}, "Android");
  if (!window_->attach())
    WARNING("Failed to attach window to renderer");
}

void SingleWindowManager::apply_window_state_update(const WindowState::List &updated, const WindowState::List &removed) {
  (void)updated;
  (void)removed;
}

std::shared_ptr<Window> SingleWindowManager::find_window_for_task(const Task::Id &task) {
  (void)task;
  return window_;
}

void SingleWindowManager::resize_task(const Task::Id &task, const anbox::graphics::Rect &rect,
                                      const std::int32_t &resize_mode) {
  (void)task;
  (void)rect;
  (void)resize_mode;
}

void SingleWindowManager::set_focused_task(const Task::Id &task) {
  (void)task;
}

void SingleWindowManager::remove_task(const Task::Id &task) {
  if (task != 0) {
    WARNING("Window with invalid task id was closed");
    return;
  }

  // FIXME easiest to terminate is sending ourself the terminate signal
  // which will be then processed by the main loop and terminate the whole
  // application.
  kill(getpid(), SIGTERM);
}
}  // namespace wm
}  // namespace anbox
