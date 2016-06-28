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

#ifndef ANBOX_DBUS_INTERFACE_H_
#define ANBOX_DBUS_INTERFACE_H_

#include <core/dbus/macros.h>

#include <string>
#include <chrono>

namespace anbox {
namespace dbus {
namespace interface {
struct Service {
    static inline std::string name() { return "org.anbox"; }
    static inline std::string path() { return "/"; }
};
struct ApplicationManager {
    static inline std::string name() { return "org.anbox.ApplicationManager"; }
    struct Methods {
        struct Install {
            static inline std::string name() { return "Install"; }
            typedef anbox::dbus::interface::ApplicationManager Interface;
            typedef void ResultType;
            static inline std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
        };
        struct Launch {
            static inline std::string name() { return "Launch"; }
            typedef anbox::dbus::interface::ApplicationManager Interface;
            typedef void ResultType;
            static inline std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
        };
    };
};
} // namespace interface
} // namespace dbus
} // namespace anbox

namespace core {
namespace dbus {
namespace traits {
template<> struct Service<anbox::dbus::interface::ApplicationManager> {
    static inline const std::string& interface_name() {
        static const std::string s{"org.anbox.ApplicationManager"};
        return s;
    }
};
} // namespace traits
} // namespace dbus
} // namespace core

#endif
