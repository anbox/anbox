/*
 * Copyright (C) 2019 Martin Unzner <martin.u@posteo.de>
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

#include "sd_bus_helpers.h"

sd_bus_vtable sdbus_vtable_create_start(uint64_t flags) {
  const sd_bus_vtable start = SD_BUS_VTABLE_START(flags);
  return start;
}

sd_bus_vtable sdbus_vtable_create_end() {
  const sd_bus_vtable end = SD_BUS_VTABLE_END;
  return end;
}

sd_bus_vtable sdbus_vtable_create_method_o(const char* member, const char* signature,
                                           const char* result,
                                           sd_bus_message_handler_t handler, size_t offset,
                                           uint64_t flags) {
  const sd_bus_vtable method_o = SD_BUS_METHOD_WITH_OFFSET(
      member, signature, result, handler, offset, flags);
  return method_o;
}

sd_bus_vtable sdbus_vtable_create_method(const char* member, const char* signature,
                                         const char* result, sd_bus_message_handler_t handler,
                                         uint64_t flags) {
  return sdbus_vtable_create_method_o(member, signature, result, handler, 0, flags);
}

sd_bus_vtable sdbus_vtable_create_signal(const char* member, const char* signature, uint64_t flags) {
  const sd_bus_vtable signal = SD_BUS_SIGNAL(member, signature, flags);
  return signal;
}

sd_bus_vtable sdbus_vtable_create_property(const char* member, const char* signature,
                                           sd_bus_property_get_t get,
                                           uint64_t flags) {
  const sd_bus_vtable property = SD_BUS_PROPERTY(member, signature, get, 0, flags);
  return property;
}

sd_bus_vtable sdbus_vtable_create_property_set(const char* member, const char* signature,
                                               sd_bus_property_get_t get,
                                               sd_bus_property_set_t set,
                                               uint64_t flags) {
  const sd_bus_vtable property = SD_BUS_WRITABLE_PROPERTY(member, signature, get, set, 0, flags);
  return property;
}

sd_bus_vtable sdbus_vtable_create_property_o(const char* member, const char* signature,
                                             size_t offset, uint64_t flags) {
  const sd_bus_vtable property_o = SD_BUS_PROPERTY(member, signature, NULL, offset, flags);
  return property_o;
}

sd_bus_vtable sdbus_vtable_create_property_o_set(const char* member, const char* signature,
                                                 sd_bus_property_set_t set, size_t offset,
                                                 uint64_t flags) {
  const sd_bus_vtable property_o = SD_BUS_WRITABLE_PROPERTY(
      member, signature, NULL, set, offset, flags);
  return property_o;
}
