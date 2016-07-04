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

#include "anbox/qemu/at_parser.h"

#include <gtest/gtest.h>

#include <algorithm>

TEST(AtParser, BasicCommands) {
    anbox::qemu::AtParser parser;

    std::string command = "ATE0Q0V1\nATE0Q0V1\n";
    std::vector<uint8_t> data;
    std::copy(command.begin(), command.end(), std::back_inserter(data));

    int commands_expected = 0;
    int commands_found = 0;
    auto assert_at_command = [&](const std::string &expected_command) {
        commands_expected++;
        return [&](const std::string &command) {
            commands_found++;
            ASSERT_STRCASEEQ(expected_command.c_str(), command.c_str());
        };
    };

    parser.register_command("E0Q0V1", assert_at_command("E0Q0V1"));
    parser.register_command("E0Q0V1", assert_at_command("E0Q0V1"));

    parser.process_data(data);

    ASSERT_EQ(commands_expected, commands_found);
}
