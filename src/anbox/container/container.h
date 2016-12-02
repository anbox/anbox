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

#ifndef ANBOX_CONTAINER_CONTAINER_H_
#define ANBOX_CONTAINER_CONTAINER_H_

#include "anbox/container/configuration.h"

#include <map>
#include <string>

namespace anbox {
namespace container {
class Container {
 public:
  virtual ~Container();

  enum class State {
    inactive,
    running,
  };

  // Start the container in background
  virtual void start(const Configuration &configuration) = 0;

  // Stop a running container
  virtual void stop() = 0;

  // Get the current container state
  virtual State state() = 0;
};
}  // namespace container
}  // namespace anbox

#endif
