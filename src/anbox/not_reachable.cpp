/*
 * Copyright (C) 2016 Canonical, Ltd.
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
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 *
 */

#include "anbox/not_reachable.h"

#include <boost/format.hpp>

anbox::util::NotReachable::NotReachable(const std::string& function,
                                        const std::string& file,
                                        std::uint32_t line)
    : std::logic_error{
          (boost::format{"Code should not be reachable: %1% in %2%:%3%"} %
           function % file % line)
              .str()} {}

void anbox::util::not_reachable(const std::string& function,
                                const std::string& file, std::uint32_t line) {
  throw NotReachable{function, file, line};
}
