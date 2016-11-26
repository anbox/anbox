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

#ifndef ANBOX_GRAPHICS_GL_RENDERER_SERVER_H_
#define ANBOX_GRAPHICS_GL_RENDERER_SERVER_H_

#include <string>
#include <memory>

namespace anbox {
namespace input {
class Manager;
} // namespace input
namespace wm {
class Manager;
} // namespace wm
namespace graphics {
class LayerComposer;
class GLRendererServer {
public:
    GLRendererServer(const std::shared_ptr<wm::Manager> &wm);
    ~GLRendererServer();

    void start();

    std::string socket_path() const;

private:
    std::string socket_path_;
    std::shared_ptr<wm::Manager> wm_;
    std::shared_ptr<LayerComposer> composer_;
};

} // namespace graphics
} // namespace anbox

#endif
