/*
* Copyright (C) 2011 The Android Open Source Project
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

#ifndef _RENDER_CONTROL_H
#define _RENDER_CONTROL_H

// Generated with emugl at build time
#include "renderControl_dec.h"

#include <memory>

class Renderer;

namespace anbox {
namespace graphics {
class LayerComposer;
}  // namespace graphics
}  // namespace anbox

void initRenderControlContext(renderControl_decoder_context_t *dec);
void registerLayerComposer(
    const std::shared_ptr<anbox::graphics::LayerComposer> &c);
void registerRenderer(const std::shared_ptr<Renderer> &r);

#endif
