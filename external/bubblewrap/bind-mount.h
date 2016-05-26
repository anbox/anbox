/* bubblewrap
 * Copyright (C) 2016 Alexander Larsson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

typedef enum {
  BIND_READONLY = (1 << 0),
  BIND_DEVICES = (1 << 2),
  BIND_RECURSIVE = (1 << 3),
} bind_option_t;

int bind_mount (int           proc_fd,
                const char   *src,
                const char   *dest,
                bind_option_t options);
