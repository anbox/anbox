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

#ifndef ANBOX_APPLICATION_MANAGER_H_
#define ANBOX_APPLICATION_MANAGER_H_

#include "anbox/android/intent.h"
#include "anbox/do_not_copy_or_move.h"
#include "anbox/graphics/rect.h"
#include "anbox/wm/stack.h"

#include <string>

#include <core/property.h>

namespace anbox {
namespace application {
class Manager : public DoNotCopyOrMove {
 public:
  virtual void launch(const android::Intent &intent,
                      const graphics::Rect &launch_bounds = graphics::Rect::Invalid,
                      const wm::Stack::Id &stack = wm::Stack::Id::Default) = 0;

  virtual core::Property<bool>& ready() = 0;
};

class RestrictedManager : public Manager {
 public:
  RestrictedManager(
      const std::shared_ptr<Manager> &other,
      const wm::Stack::Id &launch_stack = wm::Stack::Id::Invalid) :
    other_(other),
    launch_stack_(launch_stack) {}

  virtual ~RestrictedManager() {}

  void launch(const android::Intent &intent,
              const graphics::Rect &launch_bounds = graphics::Rect::Invalid,
              const wm::Stack::Id &stack = wm::Stack::Id::Default) override {
    auto selected_stack = stack;
    // If we have a static launch stack set use that one instead of
    // the one the caller gave us.
    if (launch_stack_ != wm::Stack::Id::Invalid)
      selected_stack = launch_stack_;
    other_->launch(intent, launch_bounds, selected_stack);
  }

  core::Property<bool>& ready() override { return other_->ready(); }

 private:
  std::shared_ptr<Manager> other_;
  wm::Stack::Id launch_stack_;
};
} // namespace application
} // namespace anbox

#endif
