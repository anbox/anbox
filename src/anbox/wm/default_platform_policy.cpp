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

#include "anbox/wm/default_platform_policy.h"
#include "anbox/wm/window.h"
#include "anbox/logger.h"

namespace {
class NullWindow : public anbox::wm::Window {
public:
    NullWindow(const anbox::wm::WindowState &state) :
        anbox::wm::Window(state) {
    }
};
}

namespace anbox {
namespace wm {
DefaultPlatformPolicy::DefaultPlatformPolicy() {
}

std::shared_ptr<Window> DefaultPlatformPolicy::create_window(const WindowState &state)
{
    DEBUG("");
    return std::make_shared<::NullWindow>(state);
}
} // namespace wm
} // namespace anbox
