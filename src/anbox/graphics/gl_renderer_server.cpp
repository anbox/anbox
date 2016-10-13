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
#include "anbox/graphics/window_creator.h"

#include "OpenglRender/render_api.h"

#include <boost/throw_exception.hpp>
#include <stdexcept>
#include <cstdarg>

namespace anbox {
namespace graphics {
GLRendererServer::GLRendererServer(const std::shared_ptr<WindowCreator> &window_creator) :
    window_creator_(window_creator) {

    if (utils::is_env_set("USE_HOST_GLES")) {
        // Force the host EGL/GLES libraries as translator implementation
        ::setenv("ANDROID_EGL_LIB", "libEGL.so.1", 0);
        ::setenv("ANDROID_GLESv1_LIB", "libGLESv2.so.2", 0);
        ::setenv("ANDROID_GLESv2_LIB", "libGLESv2.so.2", 0);
    } else {
        auto translator_dir = utils::prefix_dir_from_env(TRANSLATOR_INSTALL_DIR, "SNAP");
        ::setenv("ANDROID_EGL_LIB", utils::string_format("%s/libEGL_translator.so", translator_dir).c_str(), 0);
        ::setenv("ANDROID_GLESv1_LIB", utils::string_format("%s/libGLES_CM_translator.so", translator_dir).c_str(), 0);
        ::setenv("ANDROID_GLESv2_LIB", utils::string_format("%s/libGLES_V2_translator.so", translator_dir).c_str(), 0);
    }

    if (!initLibrary())
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to initialize OpenGL renderer"));

    registerSubWindowHandler(window_creator_);
}

GLRendererServer::~GLRendererServer() {
    // destroyOpenGLSubwindow();
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

    auto display_info = window_creator_->display_info();
    const auto width = display_info.horizontal_resolution;
    const auto height = display_info.vertical_resolution;

    char server_addr[256] = { 0 };
    // The width & height we supply here are the dimensions the internal framebuffer
    // will use. Making this static prevents us for now to resize the window we create
    // later for the actual display.
    if (!initOpenGLRenderer(window_creator_->native_display(), width, height, true, server_addr, sizeof(server_addr), log_funcs, logger_write))
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to setup OpenGL renderer"));

    socket_path_ = server_addr;

#if 0
    // Create the window we use for rendering the output we get from the
    // Android container. This will internally construct a Mir surface
    // and use the host EGL/GLES libraries for rendering.
    if (!showOpenGLSubwindow(0, 0, 0, width, height, width, height, 1.0f, 0))
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to setup GL based window"));
#endif
}

std::string GLRendererServer::socket_path() const {
    return socket_path_;
}
} // namespace graphics
} // namespace anbox
