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

#ifndef ANBOX_CONTAINER_LXC_CONTAINER_H_
#define ANBOX_CONTAINER_LXC_CONTAINER_H_

#include "anbox/container/container.h"

#include <string>

#include <lxc/lxccontainer.h>

namespace anbox {
namespace container {
class LxcContainer : public Container {
 public:
  LxcContainer();
  ~LxcContainer();

  void start(const Configuration &configuration) override;
  void stop() override;
  State state() override;

 private:
  void set_config_item(const std::string &key, const std::string &value);

  State state_;
  lxc_container *container_;
};
}  // namespace container
}  // namespace anbox

#endif
