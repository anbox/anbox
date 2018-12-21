/*
 * Copyright (C) 2017 Simon Fels <morphis@gravedo.de>
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

#include "anbox/ui/splash_screen.h"
#include "anbox/system_configuration.h"
#include "anbox/utils.h"
#include "anbox/logger.h"

#include <SDL2/SDL_image.h>

namespace anbox {
namespace ui {
SplashScreen::SplashScreen() {
#ifdef SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR
  // Don't disable compositing
  // Available since SDL 2.0.8
  SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
#endif

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
    const auto message = utils::string_format("Failed to initialize SDL: %s", SDL_GetError());
    BOOST_THROW_EXCEPTION(std::runtime_error(message));
  }

  const auto width = 1024, height = 768;
  window_ = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                             width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS);
  if (!window_) {
    const auto message = utils::string_format("Failed to create window: %s", SDL_GetError());
    BOOST_THROW_EXCEPTION(std::runtime_error(message));
  }

  auto surface = SDL_GetWindowSurface(window_);
  if (!surface)
    BOOST_THROW_EXCEPTION(std::runtime_error("Could not retrieve surface for our window"));

  SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 0xee, 0xee, 0xee));
  SDL_UpdateWindowSurface(window_);

  auto renderer = SDL_GetRenderer(window_);
  if (!renderer) {
    DEBUG("Window has no associated renderer yet, creating one ...");
    renderer = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
      const auto msg = utils::string_format("Could not create renderer: %s", SDL_GetError());
      BOOST_THROW_EXCEPTION(std::runtime_error(msg));
    }
  }

  const auto icon_path = utils::string_format("%s/ui/loading-screen.png", SystemConfiguration::instance().resource_dir());
  auto img = IMG_LoadTexture(renderer, icon_path.c_str());
  if (!img) {
    const auto msg = utils::string_format("Failed to create texture from %s", icon_path);
    BOOST_THROW_EXCEPTION(std::runtime_error(msg));
  }

  SDL_RenderClear(renderer);

  SDL_Rect r{0, 0, width, height};
  SDL_RenderCopy(renderer, img, nullptr, &r);
  SDL_RenderPresent(renderer);

  SDL_ShowWindow(window_);

  event_thread_ = std::thread(&SplashScreen::process_events, this);
}

SplashScreen::~SplashScreen() {
  if (event_thread_running_) {
    event_thread_running_ = false;
    if (event_thread_.joinable())
      event_thread_.join();
  }

  if (window_)
    SDL_DestroyWindow(window_);
}

void SplashScreen::process_events() {
  event_thread_running_ = true;
  while (event_thread_running_) {
    SDL_Event event;
    while (SDL_WaitEventTimeout(&event, 100)) {
      // Keep running until we're terminated
    }
  }
}
} // namespace ui
} // namespace anbox
