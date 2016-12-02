/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */
#include "anbox/common/fd.h"

#include <unistd.h>
#include <algorithm>

namespace anbox {
Fd::Fd() : Fd{invalid} {}

Fd::Fd(IntOwnedFd fd) : fd{std::make_shared<int>(fd.int_owned_fd)} {}

Fd::Fd(int raw_fd)
    : fd{new int{raw_fd},
         [](int* fd) {
           if (!fd) return;
           if (*fd > Fd::invalid) ::close(*fd);
           delete fd;
         }} {}

Fd::Fd(Fd&& other) : fd{std::move(other.fd)} {}

Fd& Fd::operator=(Fd other) {
  std::swap(fd, other.fd);
  return *this;
}

Fd::operator int() const {
  if (fd) return *fd;
  return invalid;
}
}  // namespace anbox
