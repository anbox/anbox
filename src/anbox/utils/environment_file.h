/*
 * Copyright (C) 2017 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ANBOX_UTILS_ENVIRONMENT_FILE_H_
#define ANBOX_UTILS_ENVIRONMENT_FILE_H_

#include <map>
#include <string>
#include <boost/filesystem.hpp>

namespace anbox {
namespace utils {
class EnvironmentFile {
 public:
  EnvironmentFile(const boost::filesystem::path &path);
  ~EnvironmentFile() = default;

  std::string value(const std::string &key, const std::string &default_value = "") const;

 private:
  std::map<std::string, std::string> data_;
};
} // namespace utils
} // namespace anbox

#endif
