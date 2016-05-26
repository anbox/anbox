/*
 * Copyright © 2012-2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */
#ifndef CORE_POSIX_LINUX_PROC_PROCESS_OOM_SCORE_H_
#define CORE_POSIX_LINUX_PROC_PROCESS_OOM_SCORE_H_

#include <core/posix/visibility.h>

namespace core
{
namespace posix
{
class Process;
namespace linux
{
namespace proc
{
namespace process
{
/**
 * This file displays the current score that the kernel gives to this process
 * for the purpose of selecting a process for the OOM-killer. A higher score
 * means that the process is more likely to be selected by the OOM-killer. The
 * basis for this score is the amount of memory used by the process, with
 * increases (+) or decreases (-) for factors including:
 *
 *   - whether the process creates a lot of children using fork(2) (+);
 *   - whether the process has been running a long time, or has used a lot of CPU time (-);
 *   - whether the process has a low nice value (i.e., > 0) (+);
 *   - whether the process is privileged (-); and
 *   - whether the process is making direct hardware access (-).
 *
 * The oom_score also reflects the adjustment specified by the oom_score_adj or
 * oom_adj setting for the process.
 */
struct CORE_POSIX_DLL_PUBLIC OomScore
{
    int value = 0; ///< Current OomScore as calculated by the kernel.
};

/**
 * \brief Read the OomScore for a process instance.
 * \throws std::runtime_error in case of errors.
 * \param [in] process The process to read the score for.
 * \param [out] score The destination to store the value in.
 */
CORE_POSIX_DLL_PUBLIC const posix::Process& operator>>(const posix::Process& process, OomScore& score);
}
}
}
}
}
#endif // CORE_POSIX_LINUX_PROC_PROCESS_OOM_SCORE_H_
