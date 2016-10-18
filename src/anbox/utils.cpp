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

#include <boost/filesystem.hpp>
#include <boost/throw_exception.hpp>

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include <fcntl.h>

#include "anbox/utils.h"

namespace fs = boost::filesystem;

namespace anbox {
namespace utils {

std::vector<std::string> collect_arguments(int argc, char **argv) {
    std::vector<std::string> result;
    for (int i = 1; i < argc; i++) result.push_back(argv[i]);
    return result;
}

std::string read_file_if_exists_or_throw(const std::string &file_path) {
    if (!boost::filesystem::is_regular(boost::filesystem::path(file_path)))
        BOOST_THROW_EXCEPTION(std::runtime_error("File does not exist"));

    std::ifstream file;
    file.open(file_path, std::ifstream::in);
    std::string content;
    file >> content;
    file.close();
    return content;
}

bool write_to_file(const std::string &file_path, const std::string &content) {
    std::ofstream of;
    of.open(file_path, std::ofstream::out);
    of << content;
    of.close();
    return true;
}

int write_to_fd(int fd, const char *content, ssize_t len) {
    while (len > 0) {
      const auto res = write (fd, content, len);
      if (res < 0 && errno == EINTR)
        continue;

      if (res <= 0) {
          if (res == 0) /* Unexpected short write, should not happen when writing to a file */
            errno = ENOSPC;
          return -1;
      }

      len -= res;
      content += res;
    }
    return 0;
}

int write_file_at(int dirfd, const char *path, const char *content) {
    const auto fd = openat (dirfd, path, O_RDWR | O_CLOEXEC, 0);
    if (fd == -1)
      return -1;

    auto res = 0;
    if (content)
        res = write_to_fd (fd, content, strlen (content));

    const auto errsv = errno;
    close (fd);
    errno = errsv;

    return res;
}

bool string_starts_with(const std::string &text, const std::string &prefix) {
    return text.compare(0, prefix.size(), prefix) == 0;
}

std::string hex_dump(const uint8_t *data, uint32_t size) {
    unsigned char buff[17];
    const uint8_t *pc = data;
    std::stringstream buffer;

    if (size == 0) {
        buffer << "NULL" << std::endl;
        return buffer.str();
    }

    uint32_t i;
    for (i = 0; i < size; i++) {
        if ((i % 16) == 0) {
            if (i != 0)
                buffer << string_format("  %s", buff) << std::endl;

            buffer << string_format("%02x   ", i);
        }

        buffer << string_format(" %02x", static_cast<int>(pc[i]));

        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    while ((i % 16) != 0) {
        buffer << "   ";
        i++;
    }

    buffer << string_format("  %s", buff) << std::endl;

    return buffer.str();
}

std::string get_env_value(const std::string &name, const std::string &default_value) {
    char *value = getenv(name.c_str());
    if (!value)
        return default_value;
    return std::string(value);
}

bool is_env_set(const std::string &name) {
    return getenv(name.c_str()) != nullptr;
}

void ensure_paths(const std::vector<std::string> &paths) {
    for (const auto &path: paths) {
        if (!fs::is_directory(fs::path(path)))
            fs::create_directories(fs::path(path));
    }
}

std::string prefix_dir_from_env(const std::string &path, const std::string &env_var) {
    static auto snap_data_path = anbox::utils::get_env_value(env_var, "");
    auto result = path;
    if (!snap_data_path.empty())
        result = anbox::utils::string_format("%s%s", snap_data_path, path);
    return result;
}

} // namespace utils
} // namespace anbox
