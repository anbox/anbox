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

#include <sys/stat.h>

#include <iostream>
#include <fstream>
#include <cstdio>

#include <boost/exception/errinfo_errno.hpp>

#include "anbox/network/published_socket_connector.h"
#include "anbox/network/connection_context.h"

namespace {
bool socket_file_exists(std::string const& filename)
{
    struct stat statbuf;
    bool exists = (0 == stat(filename.c_str(), &statbuf));
    /* Avoid removing non-socket files */
    bool is_socket_type = (statbuf.st_mode & S_IFMT) == S_IFSOCK;
    return exists && is_socket_type;
}

bool socket_exists(std::string const& socket_name)
{
    try
    {
        std::string socket_path{socket_name};

        /* In case an abstract socket name exists with the same name*/
        socket_path.insert(std::begin(socket_path), ' ');

        /* If the name is contained in this table, it signifies
         * a process is truly using that socket connection
         */
        std::ifstream socket_names_file("/proc/net/unix");
        std::string line;
        while (std::getline(socket_names_file, line))
        {
            auto index = line.find(socket_path);
            /* check for complete match */
            if (index != std::string::npos &&
                (index + socket_path.length()) == line.length())
            {
                return true;
            }
        }
    }
    catch (...)
    {
        /* Assume the socket exists */
        return true;
    }
    return false;
}

std::string remove_if_stale(std::string const& socket_name)
{
    if (socket_file_exists(socket_name) && !socket_exists(socket_name))
    {
        if (std::remove(socket_name.c_str()) != 0)
        {
            BOOST_THROW_EXCEPTION(
                    boost::enable_error_info(
                            std::runtime_error("Failed removing stale socket file")) << boost::errinfo_errno(errno));
        }
    }
    return socket_name;
}
}

namespace anbox {
namespace network {
PublishedSocketConnector::PublishedSocketConnector(
        const std::string& socket_file,
        const std::shared_ptr<Runtime> &rt,
        const std::shared_ptr<ConnectionCreator> &connection_creator) :
    socket_file_(remove_if_stale(socket_file)),
    runtime_(rt),
    connection_creator_(connection_creator),
    acceptor_(rt->service(), socket_file_) {

    start_accept();
}

PublishedSocketConnector::~PublishedSocketConnector() {
}

void PublishedSocketConnector::start_accept() {
    auto socket = std::make_shared<boost::asio::local::stream_protocol::socket>(runtime_->service());

    acceptor_.async_accept(
        *socket,
        [this, socket](boost::system::error_code const& err) {
            on_new_connection(socket, err);
        });
}

void PublishedSocketConnector::on_new_connection(
        std::shared_ptr<boost::asio::local::stream_protocol::socket> const& socket,
        boost::system::error_code const& err) {

    if (!err)
        connection_creator_->create_connection_for(socket);

    start_accept();
}
} // namespace network
} // namespace anbox
