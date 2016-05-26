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

#ifndef ANBOX_SUPPORT_HWCONTROL_MESSAGE_PROCESSOR_H_
#define ANBOX_SUPPORT_HWCONTROL_MESSAGE_PROCESSOR_H_

#include <memory>

#include <boost/asio.hpp>

#include "anbox/runtime.h"
#include "anbox/network/message_processor.h"
#include "anbox/network/socket_messenger.h"
#include "anbox/network/socket_connection.h"

namespace anbox {
namespace support {
class HwControlMessageProcessor : public network::MessageProcessor {
public:
    HwControlMessageProcessor(const std::shared_ptr<network::SocketMessenger> &messenger);
    ~HwControlMessageProcessor();

    bool process_data(const std::vector<std::uint8_t> &data) override;

private:
    void process_commands();

    std::shared_ptr<network::SocketMessenger> messenger_;
    std::vector<std::uint8_t> buffer_;
};
} // namespace graphics
} // namespace anbox

#endif
