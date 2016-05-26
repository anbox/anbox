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

#include "mir_support/shared_state.h"

namespace mir {
namespace support {

std::shared_ptr<SharedState> SharedState::get() {
    static auto instance = std::make_shared<SharedState>();
    return instance;
}

SharedState::SharedState() :
    connection_(nullptr) {
}

SharedState::~SharedState() {
    release_connection();
}

void SharedState::ensure_connection() {
    if (connection_)
        return;

    auto xdg_runtime_dir = ::getenv("XDG_RUNTIME_DIR");
    if (!xdg_runtime_dir)
        throw std::runtime_error("Failed to find XDG_RUNTIME_DIR");

    std::string socket_path = xdg_runtime_dir;
    socket_path += "/mir_socket";

    connection_ = mir_connect_sync(socket_path.c_str(), "anbox");
    if (!mir_connection_is_valid(connection_)) {
        std::string msg;
        msg += "Failed to connect with Mir server: ";
        msg += mir_connection_get_error_message(connection_);
        msg += "\n";
        throw std::runtime_error(msg.c_str());
    }
}

void SharedState::release_connection() {
    if (!connection_)
        return;

    mir_connection_release(connection_);
    connection_ = nullptr;
}

MirConnection* SharedState::connection() const {
    return connection_;
}

EGLNativeDisplayType SharedState::native_display() const {
    return mir_connection_get_egl_native_display(connection_);
}

} // namespace support
} // namespace mir
