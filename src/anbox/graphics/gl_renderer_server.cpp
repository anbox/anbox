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

#include "anbox/logger.h"
#include "anbox/graphics/gl_renderer_server.h"
#include "anbox/graphics/mir_native_window_creator.h"
#include "anbox/graphics/mir_display_connection.h"

#include "OpenglRender/render_api.h"

#include <boost/throw_exception.hpp>
#include <stdexcept>
#include <cstdarg>

namespace anbox {
namespace graphics {
GLRendererServer::GLRendererServer(const std::shared_ptr<InputChannel> &input_channel) :
    window_creator_(std::make_shared<MirNativeWindowCreator>(input_channel)) {

    // Force the host EGL/GLES libraries as translator implementation
    ::setenv("ANDROID_EGL_LIB", "libEGL.so.1", 1);
    ::setenv("ANDROID_GLESv1_LIB", "libGLESv1_CM.so.1", 1);
    ::setenv("ANDROID_GLESv2_LIB", "libGLESv2.so.2", 1);

    if (!initLibrary())
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to initialize OpenGL renderer"));

    setStreamMode(RENDER_API_STREAM_MODE_UNIX);

    registerSubWindowHandler(window_creator_);
}

GLRendererServer::~GLRendererServer() {
    destroyOpenGLSubwindow();
    stopOpenGLRenderer();
}

void logger_write(const char *format, ...) {
    char message[2048];
    va_list args;

    va_start(args, format);
    vsnprintf(message, sizeof(message) - 1, format, args);
    va_end(args);

    DEBUG("%s", message);
}

void GLRendererServer::start() {
    emugl_logger_struct log_funcs;
    log_funcs.coarse = logger_write;
    log_funcs.fine = logger_write;

    char server_addr[256] = { 0 };
    if (!initOpenGLRenderer(1080, 1920, true, server_addr, sizeof(server_addr), log_funcs, logger_write))
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to setup OpenGL renderer"));

    socket_path_ = server_addr;
    DEBUG("socket path %s", socket_path_);

    auto display = window_creator_->display();

    // Create the window we use for rendering the output we get from the
    // Android container. This will internally construct a Mir surface
    // and use the host EGL/GLES libraries for rendering.
    const auto width = display->horizontal_resolution();
    const auto height = display->vertical_resolution();
    if (!showOpenGLSubwindow(0, 0, 0, width, height, width, height, 1.0f, 0))
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to setup GL based window"));
}

std::string GLRendererServer::socket_path() const {
    return socket_path_;
}
} // namespace graphics
} // namespace anbox
