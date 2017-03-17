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
#include "anbox/ubuntu/platform_policy.h"
#include "anbox/input/device.h"
#include "anbox/input/manager.h"
#include "anbox/logger.h"
#include "anbox/ubuntu/keycode_converter.h"
#include "anbox/ubuntu/window.h"
#include "anbox/ubuntu/audio_sink.h"
#include "anbox/wm/manager.h"

#include <boost/throw_exception.hpp>

#include <signal.h>
#include <sys/types.h>
#pragma GCC diagnostic pop

namespace anbox {
namespace ubuntu {
PlatformPolicy::PlatformPolicy(
    const std::shared_ptr<input::Manager> &input_manager,
    const graphics::Rect &static_display_frame,
    bool single_window)
    : input_manager_(input_manager),
      event_thread_running_(false),
      single_window_(single_window) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0) {
    const auto message = utils::string_format("Failed to initialize SDL: %s", SDL_GetError());
    BOOST_THROW_EXCEPTION(std::runtime_error(message));
  }

  auto display_frame = graphics::Rect::Invalid;
  if (static_display_frame == graphics::Rect::Invalid) {
    for (auto n = 0; n < SDL_GetNumVideoDisplays(); n++) {
      SDL_Rect r;
      if (SDL_GetDisplayBounds(n, &r) != 0) continue;

      graphics::Rect frame{r.x, r.y, r.x + r.w, r.y + r.h};

      if (display_frame == graphics::Rect::Invalid)
        display_frame = frame;
      else
        display_frame.merge(frame);
    }

    if (display_frame == graphics::Rect::Invalid)
      BOOST_THROW_EXCEPTION(
          std::runtime_error("No valid display configuration found"));
  } else {
    display_frame = static_display_frame;
    window_size_immutable_ = true;
  }

  display_info_.horizontal_resolution = display_frame.width();
  display_info_.vertical_resolution = display_frame.height();

  pointer_ = input_manager->create_device();
  pointer_->set_name("anbox-pointer");
  pointer_->set_driver_version(1);
  pointer_->set_input_id({BUS_VIRTUAL, 2, 2, 2});
  pointer_->set_physical_location("none");
  pointer_->set_key_bit(BTN_MOUSE);
  // NOTE: We don't use REL_X/REL_Y in reality but have to specify them here
  // to allow InputFlinger to detect we're a cursor device.
  pointer_->set_rel_bit(REL_X);
  pointer_->set_rel_bit(REL_Y);
  pointer_->set_rel_bit(REL_HWHEEL);
  pointer_->set_rel_bit(REL_WHEEL);
  pointer_->set_prop_bit(INPUT_PROP_POINTER);

  keyboard_ = input_manager->create_device();
  keyboard_->set_name("anbox-keyboard");
  keyboard_->set_driver_version(1);
  keyboard_->set_input_id({BUS_VIRTUAL, 3, 3, 3});
  keyboard_->set_physical_location("none");
  keyboard_->set_key_bit(BTN_MISC);
  keyboard_->set_key_bit(KEY_OK);

  event_thread_ = std::thread(&PlatformPolicy::process_events, this);
}

PlatformPolicy::~PlatformPolicy() {
  if (event_thread_running_) {
    event_thread_running_ = false;
    event_thread_.join();
  }
}

void PlatformPolicy::set_renderer(const std::shared_ptr<Renderer> &renderer) {
  renderer_ = renderer;
}

void PlatformPolicy::set_window_manager(const std::shared_ptr<wm::Manager> &window_manager) {
  window_manager_ = window_manager;
}

void PlatformPolicy::process_events() {
  event_thread_running_ = true;

  while (event_thread_running_) {
    SDL_Event event;
    while (SDL_WaitEventTimeout(&event, 100)) {
      switch (event.type) {
        case SDL_QUIT:
          break;
        case SDL_WINDOWEVENT:
          for (auto &iter : windows_) {
            if (auto w = iter.second.lock()) {
              if (w->window_id() == event.window.windowID) {
                w->process_event(event);
                break;
              }
            }
          }
          break;
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEWHEEL:
        case SDL_KEYDOWN:
        case SDL_KEYUP:
          process_input_event(event);
          break;
        default:
          break;
      }
    }
  }
}

void PlatformPolicy::process_input_event(const SDL_Event &event) {
  std::vector<input::Event> mouse_events;
  std::vector<input::Event> keyboard_events;

  std::int32_t x = 0;
  std::int32_t y = 0;
  SDL_Window *window = nullptr;

  switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
      mouse_events.push_back({EV_KEY, BTN_LEFT, 1});
      mouse_events.push_back({EV_SYN, SYN_REPORT, 0});
      break;
    case SDL_MOUSEBUTTONUP:
      mouse_events.push_back({EV_KEY, BTN_LEFT, 0});
      mouse_events.push_back({EV_SYN, SYN_REPORT, 0});
      break;
    case SDL_MOUSEMOTION:
      if (!single_window_) {
        // As we get only absolute coordindates relative to our window we have to
        // calculate the correct position based on the current focused window
        window = SDL_GetWindowFromID(event.window.windowID);
        if (!window) break;

        SDL_GetWindowPosition(window, &x, &y);
        x += event.motion.x;
        y += event.motion.y;
      } else {
        // When running the whole Android system in a single window we don't
        // need to reacalculate and the pointer position as they are already
        // relative to our window.
        x = event.motion.x;
        y = event.motion.y;
      }

      // NOTE: Sending relative move events doesn't really work and we have
      // changes in libinputflinger to take ABS_X/ABS_Y instead for absolute
      // position events.
      mouse_events.push_back({EV_ABS, ABS_X, x});
      mouse_events.push_back({EV_ABS, ABS_Y, y});
      // We're sending relative position updates here too but they will be only
      // used by the Android side EventHub/InputReader to determine if the cursor
      // was moved. They are not used to find out the exact position.
      mouse_events.push_back({EV_REL, REL_X, event.motion.xrel});
      mouse_events.push_back({EV_REL, REL_Y, event.motion.yrel});
      mouse_events.push_back({EV_SYN, SYN_REPORT, 0});
      break;
    case SDL_MOUSEWHEEL:
      mouse_events.push_back(
          {EV_REL, REL_WHEEL, static_cast<std::int32_t>(event.wheel.y)});
      break;
    case SDL_KEYDOWN: {
      const auto code = KeycodeConverter::convert(event.key.keysym.scancode);
      if (code == KEY_RESERVED) break;
      keyboard_events.push_back({EV_KEY, code, 1});
      break;
    }
    case SDL_KEYUP: {
      const auto code = KeycodeConverter::convert(event.key.keysym.scancode);
      if (code == KEY_RESERVED) break;
      keyboard_events.push_back({EV_KEY, code, 0});
      break;
    }
    default:
      break;
  }

  if (mouse_events.size() > 0) pointer_->send_events(mouse_events);

  if (keyboard_events.size() > 0) keyboard_->send_events(keyboard_events);
}

Window::Id PlatformPolicy::next_window_id() {
  static Window::Id next_id = 0;
  return next_id++;
}

std::shared_ptr<wm::Window> PlatformPolicy::create_window(
    const anbox::wm::Task::Id &task, const anbox::graphics::Rect &frame, const std::string &title) {
  if (!renderer_) {
    ERROR("Can't create window without a renderer set");
    return nullptr;
  }

  auto id = next_window_id();
  auto w = std::make_shared<Window>(renderer_, id, task, shared_from_this(), frame, title, !window_size_immutable_);
  windows_.insert({id, w});
  return w;
}

void PlatformPolicy::window_deleted(const Window::Id &id) {
  auto w = windows_.find(id);
  if (w == windows_.end()) {
    WARNING("Got window removed event for unknown window (id %d)", id);
    return;
  }
  if (auto window = w->second.lock())
    window_manager_->remove_task(window->task());
  windows_.erase(w);
}

void PlatformPolicy::window_wants_focus(const Window::Id &id) {
  auto w = windows_.find(id);
  if (w == windows_.end()) return;

  if (auto window = w->second.lock())
    window_manager_->set_focused_task(window->task());
}

void PlatformPolicy::window_moved(const Window::Id &id, const std::int32_t &x,
                                  const std::int32_t &y) {
  auto w = windows_.find(id);
  if (w == windows_.end()) return;

  if (auto window = w->second.lock()) {
    auto new_frame = window->frame();
    new_frame.translate(x, y);
    window->update_frame(new_frame);
    window_manager_->resize_task(window->task(), new_frame, 3);
  }
}

void PlatformPolicy::window_resized(const Window::Id &id,
                                    const std::int32_t &width,
                                    const std::int32_t &height) {
  auto w = windows_.find(id);
  if (w == windows_.end()) return;

  if (auto window = w->second.lock()) {
    auto new_frame = window->frame();
    new_frame.resize(width, height);
    // We need to update the window frame in advance here as otherwise we may
    // get a movement event before we got an update of the actual layer
    // representing this window and then we're back to the original size of
    // the task.
    window->update_frame(new_frame);
    window_manager_->resize_task(window->task(), new_frame, 3);
  }
}

DisplayManager::DisplayInfo PlatformPolicy::display_info() const {
  return display_info_;
}

void PlatformPolicy::set_clipboard_data(const ClipboardData &data) {
  if (data.text.empty())
    return;
  SDL_SetClipboardText(data.text.c_str());
}

PlatformPolicy::ClipboardData PlatformPolicy::get_clipboard_data() {
  if (!SDL_HasClipboardText())
    return ClipboardData{};

  auto text = SDL_GetClipboardText();
  if (!text)
    return ClipboardData{};

  auto data = ClipboardData{text};
  SDL_free(text);
  return data;
}

std::shared_ptr<audio::Sink> PlatformPolicy::create_audio_sink() {
  return std::make_shared<AudioSink>();
}

std::shared_ptr<audio::Source> PlatformPolicy::create_audio_source() {
  ERROR("Not implemented");
  return nullptr;
}
}  // namespace wm
}  // namespace anbox
