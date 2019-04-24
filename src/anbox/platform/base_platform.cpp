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

#include "anbox/platform/base_platform.h"
#include "anbox/platform/null/platform.h"
#include "anbox/platform/sdl/platform.h"
#include "anbox/logger.h"

namespace anbox {
namespace platform {
std::shared_ptr<BasePlatform> create(const std::string &name,
                                     const std::shared_ptr<input::Manager> &input_manager,
                                     const Configuration &config) {
  if (name.empty())
    return std::make_shared<NullPlatform>();

  if (name == "sdl")
    return std::make_shared<sdl::Platform>(input_manager, config);

  WARNING("Unsupported platform '%s'", name);

  return nullptr;
}
} // namespace platform
} // namespace anbox
