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

#include "LayerManager.h"

#include <algorithm>

namespace {
std::string get_package_name(const std::string &name) {
    return name;
}

bool is_layer_blacklisted(const std::string &name) {
    static std::vector<std::string> blacklist = {
        "BootAnimation",
        "StatusBar",
        "Sprite",
        "KeyguardScrim",
        "com.android.launcher/com.android.launcher2.Launcher",
        "com.android.settings/com.android.settings.FallbackHome",
        "com.android.systemui.ImageWallpaper",
        "InputMethod",
    };
    return std::find(blacklist.begin(), blacklist.end(), name) != blacklist.end();
}
}

std::shared_ptr<LayerManager> LayerManager::get() {
    static auto self = std::shared_ptr<LayerManager>{new LayerManager};
    return self;
}

LayerManager::LayerManager() {
}

LayerManager::~LayerManager() {
}

void LayerManager::post_layer(const LayerInfo &layer) {
    if (is_layer_blacklisted(layer.name)) {
        printf("Ignoring blacklisted layer '%s'\n", layer.name.c_str());
        return;
    }

    FrameBufferWindow *window = nullptr;
    auto l = layers_.find(layer.name);
    if (l != layers_.end()) {
        printf("Using existing layer '%s'\n", layer.name.c_str());
        window = l->second.window;
        l->second.updated = true;
    }
    else {
        printf("New layer '%s' {%d,%d,%d,%d}\n", layer.name.c_str());
        window = FrameBuffer::getFB()->createWindow(
                    layer.display_frame.left,
                    layer.display_frame.top,
                    layer.display_frame.right,
                    layer.display_frame.bottom);
        if (!window) {
            printf("Failed to create window for layer '%s'\n", layer.name.c_str());
            return;
        }
        layers_.insert({ layer.name, Layer{window, true}});
    }

    printf("%s: window %p buffer %d\n", __func__, window, layer.buffer_handle);

    FrameBuffer::getFB()->post(window, layer.buffer_handle);
}

void LayerManager::finish_cycle() {
    for (auto iter = layers_.begin(); iter != layers_.end(); ++iter) {
        if (!iter->second.updated) {
            FrameBuffer::getFB()->destroyWindow(iter->second.window);
            layers_.erase(iter);
        }
    }
}
