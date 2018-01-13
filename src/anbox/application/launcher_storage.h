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

#ifndef ANBOX_APPLICATION_LAUNCHER_STORAGE_H_
#define ANBOX_APPLICATION_LAUNCHER_STORAGE_H_

#include "anbox/application/database.h"
#include "anbox/android/intent.h"

#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace anbox {
namespace application {
class LauncherStorage {
 public:
  LauncherStorage(const boost::filesystem::path &path);
  ~LauncherStorage();

  void reset();
  void add_or_update(const Database::Item &item);
  void remove(const Database::Item &item);

 private:
  std::string clean_package_name(const std::string &package_name);
  boost::filesystem::path path_for_item(const std::string &package_name);
  boost::filesystem::path path_for_item_icon(const std::string &package_name);

  boost::filesystem::path path_;
};
}  // namespace application
}  // namespace anbox

#endif
