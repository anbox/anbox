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

#ifndef DISPLAY_MANAGER_H_
#define DISPLAY_MANAGER_H_

#include <memory>

class DisplayManager {
 public:
  virtual ~DisplayManager();

  struct DisplayInfo {
    int horizontal_resolution;
    int vertical_resolution;
  };

  virtual DisplayInfo display_info() const = 0;

  static std::shared_ptr<DisplayManager> get();
};

void registerDisplayManager(const std::shared_ptr<DisplayManager> &mgr);

#endif
