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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-default"
#include "anbox/ubuntu/window.h"
#include "anbox/logger.h"
#include "anbox/wm/window_state.h"

#include <boost/throw_exception.hpp>

#if defined(MIR_SUPPORT)
#include <mir_toolkit/mir_client_library.h>
#endif

#include <SDL_syswm.h>
#pragma GCC diagnostic pop

namespace anbox {
namespace ubuntu {
Window::Id Window::Invalid{-1};

Window::Observer::~Observer() {}

Window::Window(const std::shared_ptr<Renderer> &renderer,
               const Id &id, const wm::Task::Id &task,
               const std::shared_ptr<Observer> &observer,
               const graphics::Rect &frame,
               const std::string &title,
               bool resizable)
    : wm::Window(renderer, task, frame, title),
      id_(id),
      observer_(observer),
      native_display_(0),
      native_window_(0) {
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);

  // NOTE: We don't furce GL initialization of the window as this will
  // be take care of by the Renderer when we attach to it. On EGL
  // initializing GL here will cause a surface to be created and the
  // renderer will attempt to create one too which will not work as
  // only a single surface per EGLNativeWindowType is supported.
  std::uint32_t flags = 0;
  if (resizable)
    flags |= SDL_WINDOW_RESIZABLE;

  window_ = SDL_CreateWindow(title.c_str(),
                             frame.left(), frame.top(),
                             frame.width(), frame.height(),
                             flags);
  if (!window_) {
    const auto message = utils::string_format("Failed to create window: %s", SDL_GetError());
    BOOST_THROW_EXCEPTION(std::runtime_error(message));
  }

  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);
  SDL_GetWindowWMInfo(window_, &info);
  switch (info.subsystem) {
    case SDL_SYSWM_X11:
      native_display_ = static_cast<EGLNativeDisplayType>(info.info.x11.display);
      native_window_ = static_cast<EGLNativeWindowType>(info.info.x11.window);
      break;
#if defined(MIR_SUPPORT)
    case SDL_SYSWM_MIR: {
      native_display_ = static_cast<EGLNativeDisplayType>(mir_connection_get_egl_native_display(info.info.mir.connection));
      auto buffer_stream = mir_surface_get_buffer_stream(info.info.mir.surface);
      native_window_ = reinterpret_cast<EGLNativeWindowType>(mir_buffer_stream_get_egl_native_window(buffer_stream));
      break;
    }
#endif
    default:
      ERROR("Unknown subsystem (%d)", info.subsystem);
      BOOST_THROW_EXCEPTION(std::runtime_error("SDL subsystem not suported"));
  }

  SDL_ShowWindow(window_);
}

Window::~Window() {
  if (window_) SDL_DestroyWindow(window_);
}

void Window::process_event(const SDL_Event &event) {
  switch (event.window.event) {
    case SDL_WINDOWEVENT_FOCUS_GAINED:
      if (observer_) observer_->window_wants_focus(id_);
      break;
    case SDL_WINDOWEVENT_FOCUS_LOST:
      break;
    // Not need to listen for SDL_WINDOWEVENT_RESIZED here as the
    // SDL_WINDOWEVENT_SIZE_CHANGED is always sent.
    case SDL_WINDOWEVENT_SIZE_CHANGED:
      if (observer_)
        observer_->window_resized(id_, event.window.data1, event.window.data2);
      break;
    case SDL_WINDOWEVENT_MOVED:
      if (observer_)
        observer_->window_moved(id_, event.window.data1, event.window.data2);
      break;
    case SDL_WINDOWEVENT_SHOWN:
      break;
    case SDL_WINDOWEVENT_HIDDEN:
      break;
    case SDL_WINDOWEVENT_CLOSE:
      if (observer_) observer_->window_deleted(id_);
      break;
    default:
      break;
  }
}

EGLNativeWindowType Window::native_handle() const { return native_window_; }

Window::Id Window::id() const { return id_; }

std::uint32_t Window::window_id() const { return SDL_GetWindowID(window_); }
}  // namespace bridge
}  // namespace anbox
