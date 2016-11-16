/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#ifndef ANBOX_NETWORK_MESSAGE_SENDER_H_
#define ANBOX_NETWORK_MESSAGE_SENDER_H_

#include <sys/types.h>
#include <cstddef>

namespace anbox {
namespace network {
class MessageSender {
 public:
  virtual void send(char const* data, size_t length) = 0;
  virtual ssize_t send_raw(char const* data, size_t length) = 0;

 protected:
  MessageSender() = default;
  virtual ~MessageSender() = default;
  MessageSender(MessageSender const&) = delete;
  MessageSender& operator=(MessageSender const&) = delete;
};
}  // namespace anbox
}  // namespace network

#endif
