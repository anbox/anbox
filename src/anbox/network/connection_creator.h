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

#pragma once

#include <boost/asio.hpp>

#include <memory>

#include "anbox/do_not_copy_or_move.h"
namespace anbox::network {
template <typename stream_protocol>
class ConnectionCreator : public DoNotCopyOrMove {
 public:
  virtual void create_connection_for(
      std::shared_ptr<boost::asio::basic_stream_socket<stream_protocol>> const&
          socket) = 0;
};
}
