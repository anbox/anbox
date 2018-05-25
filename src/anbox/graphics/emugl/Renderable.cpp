/*
* Copyright (C) 2016 Simon Fels <morphis@gravedo.de>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "anbox/graphics/emugl/Renderable.h"

Renderable::Renderable(const std::string &name, const std::uint32_t &buffer, float alpha,
                       const anbox::graphics::Rect &screen_position,
                       const anbox::graphics::Rect &crop,
                       const glm::mat4 &transformation)
    : name_(name),
      buffer_(buffer),
      screen_position_(screen_position),
      crop_(crop),
      transformation_(transformation),
      alpha_(alpha) {}

Renderable::~Renderable() {}

std::string Renderable::name() const { return name_; }

std::uint32_t Renderable::buffer() const { return buffer_; }

anbox::graphics::Rect Renderable::screen_position() const {
  return screen_position_;
}

anbox::graphics::Rect Renderable::crop() const { return crop_; }

glm::mat4 Renderable::transformation() const { return transformation_; }

float Renderable::alpha() const { return alpha_; }

void Renderable::set_screen_position(
    const anbox::graphics::Rect &screen_position) {
  screen_position_ = screen_position;
}

std::ostream &operator<<(std::ostream &out, const Renderable &r) {
  return out << "{ name " << r.name() << " buffer " << r.buffer()
             << " screen position " << r.screen_position() << " crop "
             << r.crop() << " alpha " << r.alpha();
}
