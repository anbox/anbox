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

#ifndef MIR_SUPPORT_SHARED_STATE_H_
#define MIR_SUPPORT_SHARED_STATE_H_

#define MIR_EGL_PLATFORM

#include <mirclient/mir_toolkit/mir_client_library.h>

#include <EGL/egl.h>

#include <memory>

namespace mir {
namespace support {
class SharedState {
public:
    static std::shared_ptr<SharedState> get();

    SharedState();
    ~SharedState();

    void ensure_connection();
    void release_connection();

    MirConnection* connection() const;
    EGLNativeDisplayType native_display() const;

private:
    MirConnection *connection_;
};
} // namespace support
} // namespace mir

#endif
