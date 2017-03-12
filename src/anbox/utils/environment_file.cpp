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

#include "anbox/utils/environment_file.h"
#include "anbox/utils.h"

#include <fstream>

namespace anbox {
namespace utils {
EnvironmentFile::EnvironmentFile(const boost::filesystem::path &path) {
  std::ifstream in(path.string());
  std::string line;
  while (std::getline(in, line)) {
    auto tokens = utils::string_split(line, '=');
    if (tokens.size() != 2)
      continue;
    data_[tokens[0]] = utils::strip_surrounding_quotes(tokens[1]);
  }
}

std::string EnvironmentFile::value(const std::string &key, const std::string &default_value) const {
  auto iter = data_.find(key);
  if (iter == data_.end())
    return default_value;
  return iter->second;
}
} // namespace utils
} // namespace anbox
