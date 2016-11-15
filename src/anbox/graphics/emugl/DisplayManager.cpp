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

#include "DisplayManager.h"

namespace {
std::shared_ptr<DisplayManager> display_mgr;

class NullDisplayManager : public DisplayManager {
public:
    DisplayInfo display_info() const override {
        return {1280, 720};
    }
};
}

DisplayManager::~DisplayManager() {
}

std::shared_ptr<DisplayManager> DisplayManager::get() {
    if (!display_mgr)
        display_mgr = std::make_shared<NullDisplayManager>();
    return display_mgr;
}

void registerDisplayManager(const std::shared_ptr<DisplayManager> &mgr) {
    display_mgr = mgr;
}
