/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 *
 */

#ifndef ANBOX_COMMON_DISPATCHER_H_
#define ANBOX_COMMON_DISPATCHER_H_

#include "anbox/do_not_copy_or_move.h"
#include "anbox/runtime.h"

#include <memory>

namespace anbox {
namespace common {
class Dispatcher : public DoNotCopyOrMove {
 public:
  typedef std::function<void()> Task;
  virtual void dispatch(const Task& task) = 0;

 protected:
  Dispatcher() = default;
};

std::shared_ptr<Dispatcher> create_dispatcher_for_runtime(
    const std::shared_ptr<Runtime>&);
}  // namespace common
}  // namespace anbox

#endif
