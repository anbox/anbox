/*
* Copyright (C) 2018 OpenBMC
*
* Taken from https://github.com/openbmc/sdbusplus/blob/master/sdbusplus/vtable.hpp
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef ANBOX_DBUS_SD_BUS_HELPERS_H_
#define ANBOX_DBUS_SD_BUS_HELPERS_H_

#include <systemd/sd-bus-vtable.h>

namespace anbox {
namespace dbus {
namespace sdbus {
namespace vtable {
constexpr sd_bus_vtable start(uint64_t flags)
{
  return SD_BUS_VTABLE_START(flags);
}

constexpr sd_bus_vtable end() {
  return SD_BUS_VTABLE_END;
}

constexpr sd_bus_vtable method_o(const char* member, const char* signature,
                                 const char* result,
                                 sd_bus_message_handler_t handler, size_t offset,
                                 uint64_t flags) {
  return sd_bus_vtable SD_BUS_METHOD_WITH_OFFSET(member, signature, result, handler, offset, flags);
}

constexpr sd_bus_vtable method(const char* member, const char* signature,
                               const char* result, sd_bus_message_handler_t handler,
                               uint64_t flags) {
  return method_o(member, signature, result, handler, 0, flags);
}

constexpr sd_bus_vtable signal(const char* member, const char* signature, uint64_t flags) {
  return sd_bus_vtable SD_BUS_SIGNAL(member, signature, flags);
}

constexpr sd_bus_vtable property(const char* member, const char* signature,
                                 sd_bus_property_get_t get,
                                 uint64_t flags) {
  return sd_bus_vtable SD_BUS_PROPERTY(member, signature, get, 0, flags);
}

constexpr sd_bus_vtable property(const char* member, const char* signature,
                                 sd_bus_property_get_t get,
                                 sd_bus_property_set_t set,
                                 uint64_t flags) {
  return sd_bus_vtable SD_BUS_WRITABLE_PROPERTY(member, signature, get, set, 0, flags);
}

constexpr sd_bus_vtable property_o(const char* member, const char* signature,
                                   size_t offset, uint64_t flags) {
  return sd_bus_vtable SD_BUS_PROPERTY(member, signature, nullptr, offset, flags);
}

constexpr sd_bus_vtable property_o(const char* member, const char* signature,
                                   sd_bus_property_set_t set, size_t offset,
                                   uint64_t flags)
{
  return sd_bus_vtable SD_BUS_WRITABLE_PROPERTY(member, signature, nullptr, set, offset, flags);
}
} // namespace vtable
} // namespace sd_bus
} // namespace dbus
} // namespace anbox

#endif
