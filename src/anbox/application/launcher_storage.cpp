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
#include "anbox/logger.h"

#include <algorithm>
#include <fstream>
#include <iostream>

namespace fs = boost::filesystem;

namespace anbox {
namespace application {
LauncherStorage::LauncherStorage(const fs::path &path,
                                 const boost::filesystem::path &icon_path) : path_(path), icon_path_(icon_path) {
}

LauncherStorage::~LauncherStorage() {}

void LauncherStorage::reset() {
  if (fs::exists(path_))
    fs::remove_all(path_);

  if (fs::exists(icon_path_))
    fs::remove_all(icon_path_);
}

std::string LauncherStorage::clean_package_name(const std::string &package_name) {
  auto cleaned_package_name = package_name;
  std::replace(cleaned_package_name.begin(), cleaned_package_name.end(), '.', '-');
  return cleaned_package_name;
}

fs::path LauncherStorage::path_for_item(const std::string &package_name) {
  return path_ / utils::string_format("anbox-%s.desktop", package_name);
}

fs::path LauncherStorage::path_for_item_icon(const std::string &package_name) {
  return icon_path_ / utils::string_format("anbox-%s.png", package_name);
}

void LauncherStorage::add_or_update(const Item &item) {
  if (!fs::exists(path_)) fs::create_directories(path_);
  if (!fs::exists(icon_path_)) fs::create_directories(icon_path_);

  auto package_name = item.package;
  std::replace(package_name.begin(), package_name.end(), '.', '-');

  std::string exec = utils::string_format("%s launch ", utils::process_get_exe_path(getpid()));

  if (!item.launch_intent.action.empty())
    exec += utils::string_format("--action=%s ", item.launch_intent.action);

  if (!item.launch_intent.type.empty())
    exec += utils::string_format("--type=%s ", item.launch_intent.type);

  if (!item.launch_intent.uri.empty())
    exec += utils::string_format("--uri=%s ", item.launch_intent.uri);

  if (!item.launch_intent.package.empty())
    exec += utils::string_format("--package=%s ", item.launch_intent.package);

  if (!item.launch_intent.component.empty())
    exec += utils::string_format("--component=%s ", item.launch_intent.component);

  const auto item_icon_path = path_for_item_icon(package_name);
  if (auto desktop_item = std::ofstream(path_for_item(package_name).string())) {
    desktop_item << "[Desktop Entry]" << std::endl
                 << "Name=" << item.package << std::endl
                 << "Exec=" << exec << std::endl
                 << "Terminal=false" << std::endl
                 << "Type=Application" << std::endl
                 << "Icon=" << item_icon_path.string() << std::endl;
  } else {
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create desktop item"));
  }

  if (auto icon = std::ofstream(item_icon_path.string()))
    icon.write(item.icon.data(), item.icon.size());
  else
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to write icon"));
}

void LauncherStorage::remove(const Item &item) {
  auto package_name = clean_package_name(item.package);

  const auto item_path = path_for_item(package_name);
  if (fs::exists(item_path))
    fs::remove(item_path);

  const auto item_icon_path = path_for_item_icon(package_name);
  if (fs::exists(item_icon_path))
    fs::remove(item_icon_path);
}

}  // namespace application
}  // namespace anbox
