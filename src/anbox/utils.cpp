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
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <fcntl.h>
#include <mntent.h>
#include <sys/stat.h>
#include <sys/types.h>

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
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
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
    const auto res = write(fd, content, len);
    if (res < 0 && errno == EINTR) continue;

    if (res <= 0) {
      if (res == 0) /* Unexpected short write, should not happen when writing to
                       a file */
        errno = ENOSPC;
      return -1;
    }

    len -= res;
    content += res;
  }
  return 0;
}

int write_file_at(int dirfd, const char *path, const char *content) {
  const auto fd = openat(dirfd, path, O_RDWR | O_CLOEXEC, 0);
  if (fd == -1) return -1;

  auto res = 0;
  if (content) res = write_to_fd(fd, content, strlen(content));

  const auto errsv = errno;
  close(fd);
  errno = errsv;

  return res;
}

bool string_starts_with(const std::string &text, const std::string &prefix) {
  return text.compare(0, prefix.size(), prefix) == 0;
}

std::vector<std::string> string_split(const std::string &text, char sep) {
  std::vector<std::string> tokens;
  return boost::algorithm::split(tokens, text, boost::is_from_range(sep, sep), boost::algorithm::token_compress_on);
}

std::string strip_surrounding_quotes(const std::string &text) {
  auto result = text;
  if (text[0] == '\"' && text[text.length() - 1] == '\"')
    result = text.substr(1, text.length() - 2);
  return result;
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
      if (i != 0) buffer << string_format("  %s", buff) << std::endl;

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

std::string get_env_value(const std::string &name,
                          const std::string &default_value) {
  char *value = getenv(name.c_str());
  if (!value) return default_value;
  return std::string(value);
}

bool is_env_set(const std::string &name) {
  return getenv(name.c_str()) != nullptr;
}

void ensure_paths(const std::vector<std::string> &paths) {
  for (const auto &path : paths) {
    if (!fs::is_directory(fs::path(path)))
      fs::create_directories(fs::path(path));
  }
}

std::string prefix_dir_from_env(const std::string &path,
                                const std::string &env_var) {
  static auto snap_data_path = anbox::utils::get_env_value(env_var, "");
  auto result = path;
  if (!snap_data_path.empty())
    result = anbox::utils::string_format("%s%s", snap_data_path, path);
  return result;
}

std::string process_get_exe_path(const pid_t &pid) {
  auto exe_path = string_format("/proc/%d/exe", pid);
  return boost::filesystem::read_symlink(exe_path).string();
}

bool is_mounted(const std::string &path) {
  FILE *mtab = nullptr;
  struct mntent *part = nullptr;
  bool is_mounted = false;
  if ((mtab = setmntent("/etc/mtab", "r")) != nullptr) {
    while ((part = getmntent(mtab)) != nullptr) {
      if ((part->mnt_fsname != nullptr) && (strcmp(part->mnt_fsname, path.c_str())) == 0)
        is_mounted = true;
    }
    endmntent(mtab);
  }
  return is_mounted;

}

std::string find_program_on_path(const std::string &name) {
  struct stat sb;
  std::string path = std::string(getenv("PATH"));
  size_t start_pos = 0, end_pos = 0;
  while ((end_pos = path.find(':', start_pos)) != std::string::npos) {
    const auto current_path = path.substr(start_pos, end_pos - start_pos) + "/" + name;
    if ((::stat(current_path.c_str(), &sb) == 0) && (sb.st_mode & S_IXOTH))
      return current_path;
    start_pos = end_pos + 1;
  }
  return "";
}
}  // namespace utils
}  // namespace anbox
