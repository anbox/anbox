/* bubblewrap
 * Copyright (C) 2016 Alexander Larsson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "config.h"

#include <sys/mount.h>

#include "utils.h"
#include "bind-mount.h"

static char *
skip_line (char *line)
{
  while (*line != 0 && *line != '\n')
    line++;

  if (*line == '\n')
    line++;

  return line;
}

static char *
skip_token (char *line, bool eat_whitespace)
{
  while (*line != ' ' && *line != '\n')
    line++;

  if (eat_whitespace && *line == ' ')
    line++;

  return line;
}

static char *
unescape_mountpoint (const char *escaped, ssize_t len)
{
  char *unescaped, *res;
  const char *end;

  if (len < 0)
    len = strlen (escaped);
  end = escaped + len;

  unescaped = res = xmalloc (len + 1);
  while (escaped < end)
    {
      if (*escaped == '\\')
        {
          *unescaped++ =
            ((escaped[1] - '0') << 6) |
            ((escaped[2] - '0') << 3) |
            ((escaped[3] - '0') << 0);
          escaped += 4;
        }
      else
        {
          *unescaped++ = *escaped++;
        }
    }
  *unescaped = 0;
  return res;
}

static char *
get_mountinfo (int         proc_fd,
               const char *mountpoint)
{
  char *line_mountpoint, *line_mountpoint_end;
  cleanup_free char *mountinfo = NULL;
  cleanup_free char *free_me = NULL;
  char *line, *line_start;
  char *res = NULL;
  int i;

  if (mountpoint[0] != '/')
    {
      cleanup_free char *cwd = getcwd (NULL, 0);
      if (cwd == NULL)
        die_oom ();

      mountpoint = free_me = strconcat3 (cwd, "/", mountpoint);
    }

  mountinfo = load_file_at (proc_fd, "self/mountinfo");
  if (mountinfo == NULL)
    return NULL;

  line = mountinfo;

  while (*line != 0)
    {
      cleanup_free char *unescaped = NULL;

      line_start = line;
      for (i = 0; i < 4; i++)
        line = skip_token (line, TRUE);
      line_mountpoint = line;
      line = skip_token (line, FALSE);
      line_mountpoint_end = line;
      line = skip_line (line);

      unescaped = unescape_mountpoint (line_mountpoint, line_mountpoint_end - line_mountpoint);
      if (strcmp (mountpoint, unescaped) == 0)
        {
          res = line_start;
          line[-1] = 0;
          /* Keep going, because we want to return the *last* match */
        }
    }

  if (res)
    return xstrdup (res);
  return NULL;
}

static unsigned long
get_mountflags (int         proc_fd,
                const char *mountpoint)
{
  cleanup_free char *line = NULL;
  char *token, *end_token;
  int i;
  unsigned long flags = 0;
  static const struct  { int   flag;
                         char *name;
  } flags_data[] = {
    { 0, "rw" },
    { MS_RDONLY, "ro" },
    { MS_NOSUID, "nosuid" },
    { MS_NODEV, "nodev" },
    { MS_NOEXEC, "noexec" },
    { MS_NOATIME, "noatime" },
    { MS_NODIRATIME, "nodiratime" },
    { MS_RELATIME, "relatime" },
    { 0, NULL }
  };

  line = get_mountinfo (proc_fd, mountpoint);
  if (line == NULL)
    return 0;

  token = line;
  for (i = 0; i < 5; i++)
    token = skip_token (token, TRUE);

  end_token = skip_token (token, FALSE);
  *end_token = 0;

  do
    {
      end_token = strchr (token, ',');
      if (end_token != NULL)
        *end_token = 0;

      for (i = 0; flags_data[i].name != NULL; i++)
        if (strcmp (token, flags_data[i].name) == 0)
          flags |= flags_data[i].flag;

      if (end_token)
        token = end_token + 1;
      else
        token = NULL;
    }
  while (token != NULL);

  return flags;
}


static char **
get_submounts (int         proc_fd,
               const char *parent_mount)
{
  char *mountpoint, *mountpoint_end;
  char **submounts;
  int i, n_submounts, submounts_size;
  cleanup_free char *mountinfo = NULL;
  char *line;

  mountinfo = load_file_at (proc_fd, "self/mountinfo");
  if (mountinfo == NULL)
    return NULL;

  submounts_size = 8;
  n_submounts = 0;
  submounts = xmalloc (sizeof (char *) * submounts_size);

  line = mountinfo;

  while (*line != 0)
    {
      cleanup_free char *unescaped = NULL;
      for (i = 0; i < 4; i++)
        line = skip_token (line, TRUE);
      mountpoint = line;
      line = skip_token (line, FALSE);
      mountpoint_end = line;
      line = skip_line (line);
      *mountpoint_end = 0;

      unescaped = unescape_mountpoint (mountpoint, -1);

      if (has_path_prefix (unescaped, parent_mount))
        {
          if (n_submounts + 1 >= submounts_size)
            {
              submounts_size *= 2;
              submounts = xrealloc (submounts, sizeof (char *) * submounts_size);
            }
          submounts[n_submounts++] = xstrdup (unescaped);
        }
    }

  submounts[n_submounts] = NULL;

  return submounts;
}

int
bind_mount (int           proc_fd,
            const char   *src,
            const char   *dest,
            bind_option_t options)
{
  bool readonly = (options & BIND_READONLY) != 0;
  bool devices = (options & BIND_DEVICES) != 0;
  bool recursive = (options & BIND_RECURSIVE) != 0;
  unsigned long current_flags, new_flags;
  int i;

  if (mount (src, dest, NULL, MS_MGC_VAL | MS_BIND | (recursive ? MS_REC : 0), NULL) != 0)
    return 1;

  current_flags = get_mountflags (proc_fd, dest);

  new_flags = current_flags | (devices ? 0 : MS_NODEV) | MS_NOSUID | (readonly ? MS_RDONLY : 0);
  if (new_flags != current_flags &&
      mount ("none", dest,
             NULL, MS_MGC_VAL | MS_BIND | MS_REMOUNT | new_flags, NULL) != 0)
    return 3;

  /* We need to work around the fact that a bind mount does not apply the flags, so we need to manually
   * apply the flags to all submounts in the recursive case.
   * Note: This does not apply the flags to mounts which are later propagated into this namespace.
   */
  if (recursive)
    {
      cleanup_strv char **submounts = get_submounts (proc_fd, dest);
      if (submounts == NULL)
        return 4;

      for (i = 0; submounts[i] != NULL; i++)
        {
          current_flags = get_mountflags (proc_fd, submounts[i]);
          new_flags = current_flags | (devices ? 0 : MS_NODEV) | MS_NOSUID | (readonly ? MS_RDONLY : 0);
          if (new_flags != current_flags &&
              mount ("none", submounts[i],
                     NULL, MS_MGC_VAL | MS_BIND | MS_REMOUNT | new_flags, NULL) != 0)
            {
              /* If we can't read the mountpoint we can't remount it, but that should
                 be safe to ignore because its not something the user can access. */
              if (errno != EACCES)
                return 5;
            }
        }
    }

  return 0;
}
