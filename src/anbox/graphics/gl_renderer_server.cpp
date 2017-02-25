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
#include <boost/filesystem.hpp>
#include <cstdarg>
#include <stdexcept>

namespace {
void logger_write(const emugl::LogLevel &level, const char *format, ...) {
  (void)level;

  char message[2048];
  va_list args;

  va_start(args, format);
  vsnprintf(message, sizeof(message) - 1, format, args);
  va_end(args);

  switch (level) {
  case emugl::LogLevel::WARNING:
    WARNING("%s", message);
    break;
  case emugl::LogLevel::ERROR:
    ERROR("%s", message);
    break;
  case emugl::LogLevel::FATAL:
    FATAL("%s", message);
    break;
  case emugl::LogLevel::DEBUG:
    DEBUG("%s", message);
    break;
  case emugl::LogLevel::TRACE:
    TRACE("%s", message);
    break;
  default:
    break;
  }
}
}

namespace anbox {
namespace graphics {
GLRendererServer::GLRendererServer(const Config &config, const std::shared_ptr<wm::Manager> &wm)
    : renderer_(std::make_shared<::Renderer>()),
      wm_(wm),
      composer_(std::make_shared<LayerComposer>(renderer_, wm)) {

  std::vector<emugl::GLLibrary> gl_libs = emugl::default_gl_libraries(true);

  if (config.driver == Config::Driver::Translator) {
    DEBUG("Using GLES-to-GL translator for rendering");
    boost::filesystem::path translator_dir = utils::prefix_dir_from_env(TRANSLATOR_INSTALL_DIR, "SNAP");
    gl_libs.push_back(emugl::GLLibrary{emugl::GLLibrary::Type::EGL, (translator_dir / "libEGL_translator.so")});
    gl_libs.push_back(emugl::GLLibrary{emugl::GLLibrary::Type::GLESv1, (translator_dir / "libGLES_CM_translator.so")});
    gl_libs.push_back(emugl::GLLibrary{emugl::GLLibrary::Type::GLESv2, (translator_dir / "libGLES_V2_translator.so")});
  }

  emugl_logger_struct log_funcs;
  log_funcs.coarse = logger_write;
  log_funcs.fine = logger_write;

  if (!emugl::initialize(gl_libs, log_funcs, nullptr))
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to initialize OpenGL renderer"));

  renderer_->initialize(0);

  registerRenderer(renderer_);
  registerLayerComposer(composer_);
}

GLRendererServer::~GLRendererServer() { renderer_->finalize(); }
}  // namespace graphics
}  // namespace anbox
