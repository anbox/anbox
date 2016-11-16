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

#include "anbox/graphics/gl_renderer_server.h"
#include "anbox/graphics/emugl/RenderApi.h"
#include "anbox/graphics/emugl/RenderControl.h"
#include "anbox/graphics/emugl/Renderer.h"
#include "anbox/graphics/layer_composer.h"
#include "anbox/logger.h"
#include "anbox/wm/manager.h"

#include <boost/throw_exception.hpp>
#include <cstdarg>
#include <stdexcept>

namespace {
void logger_write(const char *format, ...) {
  char message[2048];
  va_list args;

  va_start(args, format);
  vsnprintf(message, sizeof(message) - 1, format, args);
  va_end(args);

  DEBUG("%s", message);
}
}

namespace anbox {
namespace graphics {
GLRendererServer::GLRendererServer(const std::shared_ptr<wm::Manager> &wm)
    : wm_(wm), composer_(std::make_shared<LayerComposer>(wm)) {
  if (utils::is_env_set("USE_HOST_GLES")) {
    // Force the host EGL/GLES libraries as translator implementation
    ::setenv("ANDROID_EGL_LIB", "libEGL.so.1", 0);
    ::setenv("ANDROID_GLESv1_LIB", "libGLESv2.so.2", 0);
    ::setenv("ANDROID_GLESv2_LIB", "libGLESv2.so.2", 0);
  } else {
    auto translator_dir =
        utils::prefix_dir_from_env(TRANSLATOR_INSTALL_DIR, "SNAP");
    ::setenv(
        "ANDROID_EGL_LIB",
        utils::string_format("%s/libEGL_translator.so", translator_dir).c_str(),
        0);
    ::setenv("ANDROID_GLESv1_LIB",
             utils::string_format("%s/libGLES_CM_translator.so", translator_dir)
                 .c_str(),
             0);
    ::setenv("ANDROID_GLESv2_LIB",
             utils::string_format("%s/libGLES_V2_translator.so", translator_dir)
                 .c_str(),
             0);
  }

  emugl_logger_struct log_funcs;
  log_funcs.coarse = logger_write;
  log_funcs.fine = logger_write;

  if (!emugl::initialize(log_funcs, nullptr))
    BOOST_THROW_EXCEPTION(
        std::runtime_error("Failed to initialize OpenGL renderer"));

  registerLayerComposer(composer_);
}

GLRendererServer::~GLRendererServer() {
  if (const auto r = Renderer::get()) r->finalize();
}

void GLRendererServer::start() { Renderer::initialize(0); }
}  // namespace graphics
}  // namespace anbox
