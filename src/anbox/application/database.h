/*
 * Copyright (C) 2017 Simon Fels <morphis@gravedo.de>
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

#ifndef ANBOX_APPLICATION_DATABASE_H_
#define ANBOX_APPLICATION_DATABASE_H_

#include "anbox/android/intent.h"

#include <string>
#include <map>
#include <memory>

namespace anbox {
namespace application {
class LauncherStorage;
class Database {
 public:
  struct Item {
    std::string name;
    std::string package;
    android::Intent launch_intent;
    std::vector<char> icon;

    bool valid() const { return package.length() > 0; }
  };

  static const Item Unknown;

  Database();
  ~Database();

  void store_or_update(const Item &item);
  void remove(const Item &item);

  const Item& find_by_package(const std::string &package) const;

 private:
  std::shared_ptr<LauncherStorage> storage_;
  std::map<std::string,Item> items_;
  bool done_reset = false;
};
}  // namespace application
}  // namespace anbox

#endif
