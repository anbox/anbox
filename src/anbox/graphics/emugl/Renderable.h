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

#ifndef ANBOX_GRAPHICS_EMUGL_RENDERABLE_H_
#define ANBOX_GRAPHICS_EMUGL_RENDERABLE_H_

#include "anbox/graphics/rect.h"

#include <vector>

#include <cstdint>

class Renderable
{
public:
    Renderable(const std::uint32_t &buffer,
               const anbox::graphics::Rect &screen_position);
    ~Renderable();

    std::uint32_t buffer() const;
    anbox::graphics::Rect screen_position() const;

private:
    std::uint32_t buffer_;
    anbox::graphics::Rect screen_position_;
};

typedef std::vector<Renderable> RenderableList;

#endif
