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
#include "anbox/platform/sdl/platform.h"
#include "anbox/input/device.h"
#include "anbox/input/manager.h"
#include "anbox/logger.h"
#include "anbox/platform/sdl/keycode_converter.h"
#include "anbox/platform/sdl/window.h"
#include "anbox/platform/sdl/audio_sink.h"
#include "anbox/wm/manager.h"

#include <boost/throw_exception.hpp>

#include <signal.h>
#include <sys/types.h>
#pragma GCC diagnostic pop

namespace anbox {
namespace platform {
namespace sdl {
Platform::Platform(
    const std::shared_ptr<input::Manager> &input_manager,
    const Configuration &config)
    : input_manager_(input_manager),
      event_thread_running_(false),
      config_(config) {

  // Don't block the screensaver from kicking in. It will be blocked
  // by the desktop shell already and we don't have to do this again.
  // If we would leave this enabled it will prevent systems from
  // suspending correctly.
  SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "1");

#ifdef SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR
  // Don't disable compositing
  // Available since SDL 2.0.8
  SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
#endif

#ifdef SDL_HINT_TOUCH_MOUSE_EVENTS
  // Don't emulate mouse events from touch, we're handling touch ourselves.
  // Available since SDL 2.0.10
  SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
#endif

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0) {
    const auto message = utils::string_format("Failed to initialize SDL: %s", SDL_GetError());
    BOOST_THROW_EXCEPTION(std::runtime_error(message));
  }

  auto display_frame = graphics::Rect::Invalid;
  if (config_.display_frame == graphics::Rect::Invalid) {
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
    display_frame = config_.display_frame;
    window_size_immutable_ = true;
  }

  graphics::emugl::DisplayInfo::get()->set_resolution(display_frame.width(), display_frame.height());
  display_frame_ = display_frame;

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

  touch_ = input_manager->create_device();
  touch_->set_name("anbox-touch");
  touch_->set_driver_version(1);
  touch_->set_input_id({BUS_VIRTUAL, 4, 4, 4});
  touch_->set_physical_location("none");
  touch_->set_abs_bit(ABS_MT_SLOT);
  touch_->set_abs_max(ABS_MT_SLOT, 10);
  touch_->set_abs_bit(ABS_MT_TOUCH_MAJOR);
  touch_->set_abs_max(ABS_MT_TOUCH_MAJOR, 127);
  touch_->set_abs_bit(ABS_MT_TOUCH_MINOR);
  touch_->set_abs_max(ABS_MT_TOUCH_MINOR, 127);
  touch_->set_abs_bit(ABS_MT_POSITION_X);
  touch_->set_abs_max(ABS_MT_POSITION_X, display_frame.width());
  touch_->set_abs_bit(ABS_MT_POSITION_Y);
  touch_->set_abs_max(ABS_MT_POSITION_Y, display_frame.height());
  touch_->set_abs_bit(ABS_MT_TRACKING_ID);
  touch_->set_abs_max(ABS_MT_TRACKING_ID, MAX_TRACKING_ID);
  touch_->set_prop_bit(INPUT_PROP_DIRECT);

  for (int i = 0; i < MAX_FINGERS; i++)
      touch_slots[i] = -1;

  event_thread_ = std::thread(&Platform::process_events, this);
}

Platform::~Platform() {
  if (event_thread_running_) {
    event_thread_running_ = false;
    event_thread_.join();
  }
}

void Platform::set_renderer(const std::shared_ptr<Renderer> &renderer) {
  renderer_ = renderer;
}

void Platform::set_window_manager(const std::shared_ptr<wm::Manager> &window_manager) {
  window_manager_ = window_manager;
}

void Platform::process_events() {
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
        case SDL_KEYDOWN:
        case SDL_KEYUP:
          if (keyboard_)
            process_input_event(event);
          break;
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEWHEEL:
        case SDL_FINGERDOWN:
        case SDL_FINGERUP:
        case SDL_FINGERMOTION:
          process_input_event(event);
          break;
        default:
          break;
      }
    }
  }
}

void Platform::process_input_event(const SDL_Event &event) {
  std::vector<input::Event> mouse_events;
  std::vector<input::Event> keyboard_events;
  std::vector<input::Event> touch_events;

  std::int32_t x = 0;
  std::int32_t y = 0;

  switch (event.type) {
    // Mouse
    case SDL_MOUSEBUTTONDOWN:
      if (config_.no_touch_emulation) {
        mouse_events.push_back({EV_KEY, BTN_LEFT, 1});
      } else {
        x = event.button.x;
        y = event.button.y;
        if (!adjust_coordinates(x, y))
          break;
        push_finger_down(x, y, emulated_touch_id_, touch_events);
      }
      break;
    case SDL_MOUSEBUTTONUP:
      if (config_.no_touch_emulation) {
        mouse_events.push_back({EV_KEY, BTN_LEFT, 0});
      } else {
        push_finger_up(emulated_touch_id_, touch_events);
      }
      break;
    case SDL_MOUSEMOTION:
      x = event.motion.x;
      y = event.motion.y;
      if (!adjust_coordinates(x, y))
        break;

      if (config_.no_touch_emulation) {
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
      } else {
        push_finger_motion(x, y, emulated_touch_id_, touch_events);
      }
      break;
    case SDL_MOUSEWHEEL:
      if (!config_.no_touch_emulation) {
        SDL_GetMouseState(&x, &y);
        if (!adjust_coordinates(x, y))
          break;

        mouse_events.push_back({EV_ABS, ABS_X, x});
        mouse_events.push_back({EV_ABS, ABS_Y, y});
      }
      mouse_events.push_back(
          {EV_REL, REL_WHEEL, static_cast<std::int32_t>(event.wheel.y)});
      break;
    // Keyboard
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
    // Touch screen
    case SDL_FINGERDOWN: {
      if (!calculate_touch_coordinates(event, x, y))
        break;
      push_finger_down(x, y, event.tfinger.fingerId, touch_events);

      break;
    }
    case SDL_FINGERUP: {
      push_finger_up(event.tfinger.fingerId, touch_events);
      break;
    }
	case SDL_FINGERMOTION: {

      if (!calculate_touch_coordinates(event, x, y))
        break;
      push_finger_motion(x, y, event.tfinger.fingerId, touch_events);
      break;
    }
    default:
      break;
  }

  if (mouse_events.size() > 0) {
    mouse_events.push_back({EV_SYN, SYN_REPORT, 0});      
    pointer_->send_events(mouse_events);
  }

  if (keyboard_events.size() > 0)
    keyboard_->send_events(keyboard_events);

  if (touch_events.size() > 0)
    touch_->send_events(touch_events);
}

int Platform::find_touch_slot(int id){
    for (int i = 0; i < MAX_FINGERS; i++) {
        if (touch_slots[i] == id)
          return i;
    }
    return -1;
}

void Platform::push_slot(std::vector<input::Event> &touch_events, int slot){
    if (last_slot != slot) {
        touch_events.push_back({EV_ABS, ABS_MT_SLOT, slot});
        last_slot = slot;
    }
}

void Platform::push_finger_down(int x, int y, int finger_id, std::vector<input::Event> &touch_events){
    int slot = find_touch_slot(-1);
    if (slot == -1) {
        DEBUG("no free slot!");
        return;
    }
    touch_slots[slot] = finger_id;
    push_slot(touch_events, slot);
    touch_events.push_back({EV_ABS, ABS_MT_TRACKING_ID, static_cast<std::int32_t>(finger_id % MAX_TRACKING_ID + 1)});
    touch_events.push_back({EV_ABS, ABS_MT_POSITION_X, x});
    touch_events.push_back({EV_ABS, ABS_MT_POSITION_Y, y});
    touch_events.push_back({EV_SYN, SYN_REPORT, 0});
}

void Platform::push_finger_up(int finger_id, std::vector<input::Event> &touch_events){
    int slot = find_touch_slot(finger_id);
    if (slot == -1) 
      return;
    push_slot(touch_events, slot);
    touch_events.push_back({EV_ABS, ABS_MT_TRACKING_ID, -1});
    touch_events.push_back({EV_SYN, SYN_REPORT, 0});
    touch_slots[slot] = -1;
}

void Platform::push_finger_motion(int x, int y, int finger_id, std::vector<input::Event> &touch_events){
    int slot = find_touch_slot(finger_id);
    if (slot == -1) 
      return;
    push_slot(touch_events, slot);
    touch_events.push_back({EV_ABS, ABS_MT_POSITION_X, x});
    touch_events.push_back({EV_ABS, ABS_MT_POSITION_Y, y});
    touch_events.push_back({EV_SYN, SYN_REPORT, 0});
}


bool Platform::adjust_coordinates(std::int32_t &x, std::int32_t &y) {
  SDL_Window *window = nullptr;

  if (!config_.single_window) {
    window = SDL_GetWindowFromID(focused_sdl_window_id_);
    return adjust_coordinates(window, x, y);
  } else {
    // When running the whole Android system in a single window we don't
    // need to reacalculate and the pointer position as they are already
    // relative to our window.
    return true;
  }
}

bool Platform::adjust_coordinates(SDL_Window *window, std::int32_t &x, std::int32_t &y) {
  std::int32_t rel_x = 0;
  std::int32_t rel_y = 0;

  if (!window) {
    return false;
  }
  // As we get only absolute coordindates relative to our window we have to
  // calculate the correct position based on the current focused window
  SDL_GetWindowPosition(window, &rel_x, &rel_y);
  x += rel_x;
  y += rel_y;
  return true;
}

bool Platform::calculate_touch_coordinates(const SDL_Event &event,
                                           std::int32_t &x,
                                           std::int32_t &y) {
  SDL_Window *window = nullptr;

  window = SDL_GetWindowFromID(focused_sdl_window_id_);
  // before SDL 2.0.7 on X11 tfinger coordinates are not normalized
  if (!SDL_VERSION_ATLEAST(2,0,7) && (event.tfinger.x > 1 || event.tfinger.y > 1)) {
    x = event.tfinger.x;
    y = event.tfinger.y;
  } else {
    if (window) {
      SDL_GetWindowSize(window, &x, &y);
      x *= event.tfinger.x;
      y *= event.tfinger.y;
    } else {
      x = display_frame_.width() * event.tfinger.x;
      y = display_frame_.height() * event.tfinger.y;
    }
  }

  if (config_.single_window) {
    // When running the whole Android system in a single window we don't
    // need to reacalculate and the pointer position as they are already
    // relative to our window.
    return true;
  } else {
    return adjust_coordinates(window, x, y);
  }
}

Window::Id Platform::next_window_id() {
  static Window::Id next_id = 0;
  return next_id++;
}

std::shared_ptr<wm::Window> Platform::create_window(
    const anbox::wm::Task::Id &task, const anbox::graphics::Rect &frame, const std::string &title) {
  if (!renderer_) {
    ERROR("Can't create window without a renderer set");
    return nullptr;
  }

  auto id = next_window_id();
  auto w = std::make_shared<Window>(renderer_, id, task, shared_from_this(), frame, title,
		  !window_size_immutable_, !config_.server_side_decoration);
  focused_sdl_window_id_ = w->window_id();
  windows_.insert({id, w});
  return w;
}

void Platform::window_deleted(const Window::Id &id) {
  auto w = windows_.find(id);
  if (w == windows_.end()) {
    WARNING("Got window removed event for unknown window (id %d)", id);
    return;
  }
  if (auto window = w->second.lock())
    window_manager_->remove_task(window->task());
  windows_.erase(w);
}

void Platform::window_wants_focus(const Window::Id &id) {
  auto w = windows_.find(id);
  if (w == windows_.end()) return;

  if (auto window = w->second.lock()) {
    focused_sdl_window_id_ = window->window_id();
    window_manager_->set_focused_task(window->task());
  }
}

void Platform::window_moved(const Window::Id &id, const std::int32_t &x,
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

void Platform::window_resized(const Window::Id &id,
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

void Platform::set_clipboard_data(const ClipboardData &data) {
  if (data.text.empty())
    return;
  SDL_SetClipboardText(data.text.c_str());
}

Platform::ClipboardData Platform::get_clipboard_data() {
  if (!SDL_HasClipboardText())
    return ClipboardData{};

  auto text = SDL_GetClipboardText();
  if (!text)
    return ClipboardData{};

  auto data = ClipboardData{text};
  SDL_free(text);
  return data;
}

std::shared_ptr<audio::Sink> Platform::create_audio_sink(const std::shared_ptr<network::LocalSocketMessenger> &messenger) {
  return std::make_shared<AudioSink>(messenger);
}

std::shared_ptr<audio::Source> Platform::create_audio_source() {
  ERROR("Not implemented");
  return nullptr;
}

bool Platform::supports_multi_window() const {
  return true;
}
} // namespace sdl
} // namespace platform
} // namespace anbox
