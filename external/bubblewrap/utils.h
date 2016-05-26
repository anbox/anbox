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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#if 1
#define __debug__(x) printf x
#else
#define __debug__(x)
#endif

#define UNUSED __attribute__((__unused__))

#define N_ELEMENTS(arr) (sizeof (arr) / sizeof ((arr)[0]))

#define TRUE 1
#define FALSE 0
typedef int bool;

#define PIPE_READ_END 0
#define PIPE_WRITE_END 1

void  die_with_error (const char *format,
                      ...) __attribute__((__noreturn__)) __attribute__((format (printf, 1, 2)));
void  die (const char *format,
           ...) __attribute__((__noreturn__));
void  die_oom (void) __attribute__((__noreturn__));
void  die_unless_label_valid (const char *label);

void *xmalloc (size_t size);
void *xcalloc (size_t size);
void *xrealloc (void  *ptr,
                size_t size);
char *xstrdup (const char *str);
void  strfreev (char **str_array);
void  xsetenv (const char *name,
               const char *value,
               int         overwrite);
void  xunsetenv (const char *name);
char *strconcat (const char *s1,
                 const char *s2);
char *strconcat3 (const char *s1,
                  const char *s2,
                  const char *s3);
char * xasprintf (const char *format,
                  ...) __attribute__((format (printf, 1, 2)));
bool  has_prefix (const char *str,
                  const char *prefix);
bool  has_path_prefix (const char *str,
                       const char *prefix);
int   fdwalk (int                     proc_fd,
              int                     (*cb)(void *data,
                                  int fd),
              void                   *data);
char *load_file_data (int     fd,
                      size_t *size);
char *load_file_at (int         dirfd,
                    const char *path);
int   write_file_at (int         dirfd,
                     const char *path,
                     const char *content);
int   write_to_fd (int         fd,
                   const char *content,
                   ssize_t     len);
int   copy_file_data (int sfd,
                      int dfd);
int   copy_file (const char *src_path,
                 const char *dst_path,
                 mode_t      mode);
int   create_file (const char *path,
                   mode_t      mode,
                   const char *content);
int   ensure_file (const char *path,
                   mode_t      mode);
int   get_file_mode (const char *pathname);
int   mkdir_with_parents (const char *pathname,
                          int         mode,
                          bool        create_last);

/* syscall wrappers */
int   raw_clone (unsigned long flags,
                 void         *child_stack);
int   pivot_root (const char *new_root,
                  const char *put_old);
char *label_mount (const char *opt,
                   const char *mount_label);
int   label_exec (const char *exec_label);
int   label_create_file (const char *file_label);

static inline void
cleanup_freep (void *p)
{
  void **pp = (void **) p;

  if (*pp)
    free (*pp);
}

static inline void
cleanup_strvp (void *p)
{
  void **pp = (void **) p;

  strfreev (*pp);
}

static inline void
cleanup_fdp (int *fdp)
{
  int fd;

  assert (fdp);

  fd = *fdp;
  if (fd != -1)
    (void) close (fd);
}

#define cleanup_free __attribute__((cleanup (cleanup_freep)))
#define cleanup_fd __attribute__((cleanup (cleanup_fdp)))
#define cleanup_strv __attribute__((cleanup (cleanup_strvp)))

static inline void *
steal_pointer (void *pp)
{
  void **ptr = (void **) pp;
  void *ref;

  ref = *ptr;
  *ptr = NULL;

  return ref;
}

/* type safety */
#define steal_pointer(pp) \
  (0 ? (*(pp)) : (steal_pointer) (pp))
