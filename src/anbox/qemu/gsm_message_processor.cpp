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

#include "anbox/qemu/gsm_message_processor.h"
#include "anbox/logger.h"
#include "anbox/qemu/at_parser.h"

#include <algorithm>
#include <functional>

using namespace std::placeholders;

namespace anbox {
namespace qemu {
GsmMessageProcessor::GsmMessageProcessor(
    const std::shared_ptr<network::SocketMessenger> &messenger)
    : messenger_(messenger), parser_(std::make_shared<AtParser>()) {
  auto ok_reply = [&](const std::string &) { send_reply("OK"); };

  parser_->register_command("E0Q0V1", ok_reply);
  parser_->register_command("S0=0", ok_reply);
  parser_->register_command(
      "+CTEC", std::bind(&GsmMessageProcessor::handle_ctec, this, _1));
  parser_->register_command("+CMEE=1", ok_reply);
  parser_->register_command("+CCWA=1", ok_reply);
  parser_->register_command("+CMOD=0", ok_reply);
  parser_->register_command("+CMUT=0", ok_reply);
  parser_->register_command("+CSSN=0,1", ok_reply);
  parser_->register_command("+COLP=0", ok_reply);
  parser_->register_command("+CSCS=\"HEX\"", ok_reply);
  parser_->register_command("+CUSD=1", ok_reply);
  parser_->register_command("+CGEREP=1,0", ok_reply);
  parser_->register_command(
      "+CMGF", std::bind(&GsmMessageProcessor::handle_cmgf, this, _1));
  parser_->register_command("%CPI=3", ok_reply);
  parser_->register_command("%CSTAT=1", ok_reply);
  parser_->register_command(
      "+CREG", std::bind(&GsmMessageProcessor::handle_creg, this, _1));
  parser_->register_command(
      "+CGREG", std::bind(&GsmMessageProcessor::handle_cgreg, this, _1));
  parser_->register_command(
      "+CFUN", std::bind(&GsmMessageProcessor::handle_cfun, this, _1));
}

GsmMessageProcessor::~GsmMessageProcessor() {}

bool GsmMessageProcessor::process_data(const std::vector<std::uint8_t> &data) {
  for (const auto &byte : data) buffer_.push_back(byte);

  parser_->process_data(buffer_);
  return true;
}

void GsmMessageProcessor::send_reply(const std::string &message) {
  auto reply = utils::string_format("%s\rOK\n", message);
  std::vector<std::uint8_t> data;
  std::copy(reply.begin(), reply.end(), std::back_inserter(data));
  messenger_->send(reinterpret_cast<const char *>(data.data()), data.size());
}

void GsmMessageProcessor::handle_ctec(const std::string &command) {
  if (command == "+CTEC=?")
    send_reply("+CTEC: 0,1,2,3");
  else if (command == "+CTEC?")
    send_reply(utils::string_format(
        "+CTEC: %d,%x", static_cast<unsigned int>(technology::gsm), 0x0f));
}

void GsmMessageProcessor::handle_cmgf(const std::string &command) {
  if (command == "+CMGF=0") send_reply("");
}

void GsmMessageProcessor::handle_creg(const std::string &command) {
  if (command == "+CREG=?")
    send_reply("+CREG: (0-2)");
  else if (command == "+CREG?")
    send_reply(utils::string_format("+CREF: %d,%d", 0, 0));
  else if (utils::string_starts_with(command, "+CREG="))
    send_reply("");
}

void GsmMessageProcessor::handle_cgreg(const std::string &command) {
  if (command == "+CGREG=?")
    send_reply("+CGREG: (0-2)");
  else if (command == "+CGREG?")
    send_reply(utils::string_format("+CGREG: %d,%d", 0, 0));
  else if (utils::string_starts_with(command, "+CGREG="))
    send_reply("");
}

void GsmMessageProcessor::handle_cfun(const std::string &command) {
  if (command == "+CFUN?")
    send_reply(utils::string_format("+CFUN: %d", 1));
  else if (utils::string_starts_with(command, "+CFUN="))
    send_reply("");
}
}  // namespace qemu
}  // namespace anbox
