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

#include "anbox/id_map_writer.h"
#include "anbox/not_reachable.h"
#include "anbox/utils.h"

#include <fstream>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <pwd.h>

namespace fs = boost::filesystem;

#define getid(type) ((unsigned) ((type) == GID ? getgid() : getuid()))
#define idfile(type) ((type) == GID ? "gid_map" : "uid_map")
#define idname(type) ((type) == GID ? "GID" : "UID")
#define subpath(type) ((type) == GID ? "/etc/subgid" : "/etc/subuid")

namespace {
char *append(char **destination, const char *format, ...) {
  char *extra, *result;
  va_list args;

  va_start(args, format);
  if (vasprintf(&extra, format, args) < 0)
    BOOST_THROW_EXCEPTION(std::runtime_error(""));
  va_end(args);

  if (*destination == NULL) {
    *destination = extra;
    return extra;
  }

  if (asprintf(&result, "%s%s", *destination, extra) < 0)
      BOOST_THROW_EXCEPTION(std::runtime_error(""));
  free(*destination);
  free(extra);
  *destination = result;
  return result;
}

char *string(const char *format, ...) {
  char *result;
  va_list args;

  va_start(args, format);
  if (vasprintf(&result, format, args) < 0)
    BOOST_THROW_EXCEPTION(std::runtime_error(""));
  va_end(args);
  return result;
}

static char *getmap(pid_t pid, int type) {
  char *line = NULL, *result = NULL, *path;
  size_t size;
  unsigned count, first, lower;
  FILE *file;

  if (pid == -1)
    path = string("/proc/self/%s", idfile(type));
  else
    path = string("/proc/%d/%s", pid, idfile(type));
  if (!(file = fopen(path, "r")))
    BOOST_THROW_EXCEPTION(std::runtime_error(""));

  while (getline(&line, &size, file) >= 0) {
    if (sscanf(line, " %u %u %u", &first, &lower, &count) != 3)
      BOOST_THROW_EXCEPTION(std::runtime_error(""));
    append(&result, "%s%u:%u:%u", result ? "," : "", first, lower, count);
  }

  if (!result)
    BOOST_THROW_EXCEPTION(std::runtime_error(""));

  fclose(file);
  free(line);
  free(path);
  return result;
}

static char *mapitem(char *map, unsigned *first, unsigned *lower,
    unsigned *count) {
  ssize_t skip;

  while (map && *map && strchr(",;", *map))
    map++;
  if (map == NULL || *map == '\0')
    return NULL;
  if (sscanf(map, "%u:%u:%u%zn", first, lower, count, &skip) < 3)
    BOOST_THROW_EXCEPTION(std::runtime_error(""));
  return map + skip;
}

static char *rangeitem(char *range, unsigned *start, unsigned *length) {
  ssize_t skip;

  while (range && *range && strchr(",;", *range))
    range++;
  if (range == NULL || *range == '\0')
    return NULL;
  if (sscanf(range, "%u:%u%zn", start, length, &skip) < 2)
    BOOST_THROW_EXCEPTION(std::runtime_error(""));
  return range + skip;
}

static char *readranges(int type) {
  char *line = NULL, *range, *user;
  size_t end, size;
  struct passwd *passwd;
  unsigned length, start;
  FILE *file;

  range = string("%u:1", getid(type));
  if (!(file = fopen(subpath(type), "r")))
    return range;

  user = getenv("USER");
  user = user ? user : getenv("LOGNAME");
  user = user ? user : getlogin();
  if (!user || !(passwd = getpwnam(user)) || passwd->pw_uid != getuid()) {
    if (!(passwd = getpwuid(getuid())))
      BOOST_THROW_EXCEPTION(std::runtime_error(""));
    user = passwd->pw_name;
  }
  endpwent();

  while (getline(&line, &size, file) >= 0) {
    if (strncmp(line, user, strlen(user)))
      continue;
    if (sscanf(line + strlen(user), ":%u:%u%zn", &start, &length, &end) < 2)
      continue;
    if (strchr(":\n", line[end + strlen(user) + 1]))
      append(&range, ",%u:%u", start, length);
  }

  free(line);
  fclose(file);
  return range;
}

static char *rootdefault(int type) {
  char *cursor, *map, *result;
  unsigned count, first, last = INVALID, lower;

  cursor = map = getmap(-1, type);
  while ((cursor = mapitem(cursor, &first, &lower, &count)))
    if (last == INVALID || last < first + count - 1)
      last = first + count - 1;
  result = string("0:%u:1", last);

  cursor = map;
  while ((cursor = mapitem(cursor, &first, &lower, &count))) {
    if (first == 0) {
      if (count == 1 && first >= last)
        error(1, 0, "No unprivileged %s available\n", idname(type));
      first++, lower++, count--;
    }

    if (last <= first + count - 1 && count > 0)
      count--;

    if (count > 0)
      append(&result, "%s%u:%u:%u", result ? "," : "", first, first, count);
  }

  free(map);
  return result;
}

static char *userdefault(int type) {
  char *cursor, *map, *range, *result = NULL;
  unsigned count, first, index = 0, length, lower, start;

  if (geteuid() != 0)
    return string("0:%u:1", getid(type));

  map = getmap(-1, type);
  range = readranges(type);

  while ((range = rangeitem(range, &start, &length))) {
    cursor = map;
    while ((cursor = mapitem(cursor, &first, &lower, &count))) {
      if (start + length <= first || first + count <= start)
        continue;
      if (first + count < start + length)
        length = start - first + count;
      if (start < first) {
        index += first - start;
        length -= first - start;
        start = first;
      }
      append(&result, "%s%u:%u:%u", result ? "," : "", index, start, length);
      index += length;
    }
  }

  free(map);
  free(range);
  return result;
}

static void validate(char *range, unsigned first, unsigned count) {
  unsigned length, start;

  while ((range = rangeitem(range, &start, &length)))
    if (first < start + length && start < first + count) {
      if (first < start)
        validate(range, first, start - first);
      if (first + count > start + length)
        validate(range, start + length, first + count - start - length);
      return;
    }
  error(1, 0, "Cannot map onto IDs that are not delegated to you");
}

static void verifymap(char *map, char *range) {
  unsigned count, first, lower;

  while ((map = mapitem(map, &first, &lower, &count)))
    validate(range, lower, count);
}

static void writemap(pid_t pid, int type, char *map) {
  char *path, *range, *text = NULL;
  int fd;
  unsigned count, first, lower;

  if (!map) {
    map = (getuid() == 0 ? rootdefault : userdefault)(type);
  } else if (getuid() != 0) {
    range = readranges(type);
    verifymap(map, range);
    free(range);
  }

  while ((map = mapitem(map, &first, &lower, &count)))
    append(&text, "%u %u %u\n", first, lower, count);

  path = string("/proc/%d/%s", pid, idfile(type));
  if ((fd = open(path, O_WRONLY)) < 0)
    error(1, 0, "Failed to set container %s map", idname(type));
  else if (write(fd, text, strlen(text)) != (ssize_t) strlen(text))
    error(1, 0, "Failed to set container %s map", idname(type));

  close(fd);
  free(path);
  free(text);
}
}

namespace anbox {
IdMapWriter::IdMapWriter(const Type &type, const pid_t &pid) :
    type_(type),
    pid_(pid) {
}

IdMapWriter::~IdMapWriter() {
}

void IdMapWriter::apply() {
    const auto map = retrieve_mappings();
}

} // namespace anbox
