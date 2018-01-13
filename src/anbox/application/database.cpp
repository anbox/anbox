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

#include "anbox/application/database.h"
#include "anbox/application/launcher_storage.h"
#include "anbox/system_configuration.h"
#include "anbox/logger.h"

namespace anbox {
namespace application {
const Database::Item Database::Unknown{};

Database::Database() :
  storage_(std::make_shared<LauncherStorage>(SystemConfiguration::instance().application_item_dir())) {}

Database::~Database() {}

void Database::store_or_update(const Item &item) {
  if (!done_reset) {
    storage_->reset();
    done_reset = true;
  }
  storage_->add_or_update(item);
  items_[item.package] = item;

  // We don't need to store the icon data anymore at this point as the
  // launcher is already stored it on the disk.
  items_[item.package].icon.clear();
}

void Database::remove(const Item &item) {
  auto iter = items_.find(item.package);
  if (iter == items_.end())
    return;
  storage_->remove(item);
  items_.erase(iter);
}

const Database::Item& Database::find_by_package(const std::string &package) const {
  auto iter = items_.find(package);
  if (iter == items_.end())
    return Unknown;
  return iter->second;
}
}  // namespace application
}  // namespace anbox
