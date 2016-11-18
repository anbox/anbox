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

#ifndef LAYER_MANAGER_H_
#define LAYER_MANAGER_H_

#include <memory>

#include <map>
#include <vector>
#include <string>

#include "Renderer.h"

struct LayerRect {
    int left;
    int top;
    int right;
    int bottom;
};

struct LayerInfo {
    std::string name;
    LayerRect source_crop;
    LayerRect display_frame;
    HandleType buffer_handle;
};

class LayerManager {
public:
    static std::shared_ptr<LayerManager> get();

    ~LayerManager();

    void post_layer(const LayerInfo &layer);
    void finish_cycle();

private:
    LayerManager();

    struct Layer {
        RendererWindow *window;
        bool updated;
    };

    std::map<std::string,Layer> layers_;
};

#endif
