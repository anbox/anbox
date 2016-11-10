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

#include <boost/algorithm/string.hpp>

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

struct ActivityInfo {
    ActivityInfo(const std::string &name) :
        name(name) {
        std::vector<std::string> parts;
        boost::split(parts, name, boost::is_any_of("/"));
        if (parts.size() > 1)
            package = parts[0];
    }

    std::string package;
    std::string name;
};

bool from_same_package(const std::string &a, const std::string &b) {
    return ActivityInfo(a).package == ActivityInfo(b).package;
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
    if (is_layer_blacklisted(layer.name))
        return;

    FrameBufferWindow *window = nullptr;
    for (auto &l : layers_) {
        if (l.first == layer.name || from_same_package(l.first, layer.name)) {
            window = l.second.window;
            l.second.updated = true;
        }
    }

    auto width = layer.display_frame.right - layer.display_frame.left;
    auto height = layer.display_frame.bottom - layer.display_frame.top;

    if (!window) {

        window = FrameBuffer::getFB()->createWindow(layer.display_frame.left,
                                                    layer.display_frame.top,
                                                    width, height);
        if (!window) {
            printf("Failed to create window for layer '%s'\n", layer.name.c_str());
            return;
        }
        layers_.insert({ layer.name, Layer{window, true}});
    }

    FrameBuffer::getFB()->updateWindow(window,
                                       layer.display_frame.left,
                                       layer.display_frame.top,
                                       width, height);
    FrameBuffer::getFB()->post(window, layer.buffer_handle);
}

void LayerManager::finish_cycle() {
    for (auto iter = layers_.begin(); iter != layers_.end(); ++iter) {
        if (!iter->second.updated) {
            FrameBuffer::getFB()->destroyWindow(iter->second.window);
            layers_.erase(iter);
        }
    }

    for (auto &layer : layers_)
        layer.second.updated = false;
}
