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
#ifndef CORE_POSIX_LINUX_PROC_PROCESS_OOM_ADJ_H_
#define CORE_POSIX_LINUX_PROC_PROCESS_OOM_ADJ_H_

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
 * This file can be used to adjust the score used to select which process
 * should be killed in an out-of-memory (OOM) situation. The kernel uses this
 * value for a bit-shift operation of the process's oom_score value: valid
 * values are in the range -16 to +15, plus the special value -17, which disables
 * OOM-killing altogether for this process. A positive score increases the
 * likelihood of this process being killed by the OOM-killer; a negative score
 * decreases the likelihood.
 *
 * The default value for this file is 0; a new process inherits its parent's
 * oom_adj setting. A process must be privileged (CAP_SYS_RESOURCE) to update
 * this file.
 *
 * Since Linux 2.6.36, use of this file is deprecated in favor of
 * /proc/[pid]/oom_score_adj.
 */
struct CORE_POSIX_DLL_PUBLIC OomAdj
{
    /**
     * @brief Returns the value that makes a process "invisible" to the oom killer.
     * @return Returns the value that makes a process "invisible" to the oom killer.
     */
    static int disable_value();

    /**
     * @brief Returns the minimum valid value.
     * @return The minimum valid value that the OomAdj can be set to.
     */
    static int min_value();

    /**
     * @brief Returns the maximum valid value.
     * @return The maximum valid value that the OomAdj can be set to.
     */
    static int max_value();

    /**
     * @brief is_valid checks whether the contained value is within the predefined bounds.
     * @return true iff min_value() <= value <= max_value().
     */
    inline bool is_valid() const
    {
        return (disable_value() <= value) && (value <= max_value());
    }

    /**
     * @brief Current value.
     */
    int value;
};

/**
 * \brief Read the OomAdj value for a process instance.
 * \throws std::runtime_error in case of errors.
 * \param [in] process The process to read the score for.
 * \param [out] adj The destination to store the value in.
 */
CORE_POSIX_DLL_PUBLIC const posix::Process& operator>>(const posix::Process& process, OomAdj& adj);

/**
 * \brief Write the OomAdj value for a process instance.
 * \throw std::runtime_error in case of errors and std::logic_error if score_adj.is_valid() returns false.
 * \param [in] process The process to write the score for.
 * \param [in] adj The new value to store.
 */
CORE_POSIX_DLL_PUBLIC const posix::Process& operator<<(const posix::Process& process,
                                 const OomAdj& adj);
}
}
}
}
}
#endif // CORE_POSIX_LINUX_PROC_PROCESS_OOM_ADJ_H_
