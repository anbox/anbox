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

#include "anbox/application/launcher_storage.h"
#include "anbox/utils.h"

#include <algorithm>
#include <fstream>
#include <iostream>

namespace fs = boost::filesystem;

namespace anbox {
namespace application {
LauncherStorage::LauncherStorage(const fs::path &path) : path_(path) {}

LauncherStorage::~LauncherStorage() {}

void LauncherStorage::add(const Item &item) {
  if (!fs::exists(path_)) fs::create_directories(path_);

  auto package_name = item.package;
  std::replace(package_name.begin(), package_name.end(), '.', '-');

  const auto item_path =
      path_ / utils::string_format("anbox-%s.desktop", package_name);
  std::string exec = "anbox launch ";

  if (!item.launch_intent.action.empty())
    exec += utils::string_format("--action=%s ", item.launch_intent.action);

  if (!item.launch_intent.type.empty())
    exec += utils::string_format("--type=%s ", item.launch_intent.type);

  if (!item.launch_intent.uri.empty())
    exec += utils::string_format("--uri=%s ", item.launch_intent.uri);

  if (!item.launch_intent.package.empty())
    exec += utils::string_format("--package=%s ", item.launch_intent.package);

  if (!item.launch_intent.component.empty())
    exec +=
        utils::string_format("--component=%s ", item.launch_intent.component);

  std::ofstream f(item_path.string());
  f << "[Desktop Entry]" << std::endl
    << "Name=" << item.package << std::endl
    << "Exec=" << exec << std::endl
    << "Terminal=false" << std::endl
    << "Type=Application" << std::endl
    << "Encoding=UTF-8" << std::endl;
  f.close();
}
}  // namespace application
}  // namespace anbox
