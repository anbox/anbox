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

#include "anbox/wm/manager.h"
#include "anbox/wm/platform_policy.h"
#include "anbox/logger.h"

namespace anbox {
namespace wm {
Manager::Manager(const std::shared_ptr<PlatformPolicy> &platform) :
    platform_(platform) {
}

Manager::~Manager() {
}

void Manager::apply_window_state_update(const WindowState::List &updated,
                                        const WindowState::List &removed)
{
    DEBUG("updated %d removed %d", updated.size(), removed.size());

    // Base on the update we get from the Android WindowManagerService we will create
    // different window instances with the properties supplied. Incoming layer updates
    // from SurfaceFlinger will be mapped later into those windows and eventually
    // composited there via GLES (e.g. for popups, ..)

    for (const auto &window : updated) {
        auto w = windows_.find(window.task());
        if (w != windows_.end()) {
            DEBUG("Found existing window for task %d", window.task());
            w->second->update_state(window);
            continue;
        }
        DEBUG("Found new window for task %d", window.task());
        auto platform_window = platform_->create_window(window);
        platform_window->attach();
        windows_.insert({window.task(), platform_window});
    }

    for (const auto &window : removed) {
        auto w = windows_.find(window.task());
        if (w == windows_.end()) {
            WARNING("Got remove request for window we don't know about (task id %d)", window.task());
            continue;
        }
        DEBUG("Removing window for task %d", window.task());
        auto platform_window = w->second;
        platform_window->release();
        windows_.erase(w);
    }
}
} // namespace wm
} // namespace anbox
