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

#ifndef ANBOX_CONTAINER_CLIENT_H_
#define ANBOX_CONTAINER_CLIENT_H_

#include "anbox/network/local_socket_messenger.h"
#include "anbox/runtime.h"

namespace anbox {
namespace container {
class Client {
public:
    Client(const std::shared_ptr<Runtime> &rt);
    ~Client();

private:
    network::LocalSocketMessenger messenger_;
};
} // namespace container
} // namespace anbox

#endif
