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
#include "anbox/logger.h"

namespace anbox {
namespace wm {
Manager::Manager() {
}

Manager::~Manager() {
}

void Manager::apply_window_state_update(const WindowState::List &updated,
                                        const WindowState::List &removed) {
    (void) updated;
    (void) removed;

    DEBUG("updated %d removed %d", updated.size(), removed.size());
}
} // namespace wm
} // namespace anbox
