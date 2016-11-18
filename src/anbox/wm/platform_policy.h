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

#ifndef ANBOX_WM_PLATFORM_POLICY_H_
#define ANBOX_WM_PLATFORM_POLICY_H_

#include "anbox/graphics/rect.h"
#include "anbox/wm/window_state.h"

#include <memory>

namespace anbox {
namespace wm {
class Window;
class PlatformPolicy {
public:
    virtual ~PlatformPolicy();

    virtual std::shared_ptr<Window> create_window(const WindowState &state) = 0;
};
} // namespace wm
} // namespace anbox

#endif
