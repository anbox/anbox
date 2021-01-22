/*
 * Copyright © 2013-2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#pragma once

#include <mutex>

#include "anbox/network/credentials.h"
#include "anbox/network/message_receiver.h"
#include "anbox/network/message_sender.h"

namespace anbox::network {
class SocketMessenger : public MessageSender, public MessageReceiver {
 public:
  virtual Credentials creds() const = 0;
  virtual unsigned short local_port() const = 0;
  virtual void set_no_delay() = 0;
  virtual void close() = 0;
};
}
