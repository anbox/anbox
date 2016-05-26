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
#ifndef CORE_POSIX_LINUX_PROC_PROCESS_OOM_SCORE_ADJ_H_
#define CORE_POSIX_LINUX_PROC_PROCESS_OOM_SCORE_ADJ_H_

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
 * This file can be used to adjust the badness heuristic used to select which
 * process gets killed in out-of-memory conditions.
 *
 * The badness heuristic assigns a value to each candidate task ranging from 0
 * (never kill) to 1000 (always kill) to determine which process is targeted.
 * The units are roughly a proportion along that range of allowed memory the
 * process may allocate from, based on an estimation of its current memory and
 * swap use. For example, if a task is using all allowed memory, its badness
 * score will be 1000. If it is using half of its allowed memory, its score
 * will be 500.
 *
 * There is an additional factor included in the badness score: root processes are
 * given 3% extra memory over other tasks.
 *
 * The amount of "allowed" memory depends on the context in which the
 * OOM-killer was called. If it is due to the memory assigned to the allocating
 * task's cpuset being exhausted, the allowed memory represents the set of mems
 * assigned to that cpuset (see cpuset(7)). If it is due to a mempolicy's node(s)
 * being exhausted, the allowed memory represents the set of mempolicy nodes. If
 * it is due to a memory limit (or swap limit) being reached, the allowed memory
 * is that configured limit. Finally, if it is due to the entire system being out
 * of memory, the allowed memory represents all allocatable resources.
 *
 * The value of oom_score_adj is added to the badness score before it is used
 * to determine which task to kill. Acceptable values range from -1000
 * (OOM_SCORE_ADJ_MIN) to +1000 (OOM_SCORE_ADJ_MAX). This allows user space to
 * control the preference for OOM-killing, ranging from always preferring a
 * certain task or completely disabling it from OOM- killing. The lowest possible
 * value, -1000, is equivalent to disabling OOM-killing entirely for that task,
 * since it will always report a badness score of 0.
 *
 * Consequently, it is very simple for user space to define the amount of
 * memory to consider for each task. Setting a oom_score_adj value of +500, for
 * example, is roughly equivalent to allowing the remainder of tasks sharing
 * the same system, cpuset, mempolicy, or memory controller resources to use at
 * least 50% more memory. A value of -500, on the other hand, would be roughly
 * equivalent to discounting 50% of the task's allowed memory from being
 * considered as scoring against the task.
 *
 * For backward compatibility with previous kernels, /proc/[pid]/oom_adj can
 * still be used to tune the badness score. Its value is scaled linearly with
 * oom_score_adj.
 *
 * Writing to /proc/[pid]/oom_score_adj or /proc/[pid]/oom_adj will change the
 * other with its scaled value.
 */
struct CORE_POSIX_DLL_PUBLIC OomScoreAdj
{
    /**
     * @brief Returns the minimum valid value.
     * @return The minimum valid value that the Oom Score Adj can be set to.
     */
    static int min_value();

    /**
     * @brief Returns the maximum valid value.
     * @return The maximum valid value that the Oom Score Adj can be set to.
     */
    static int max_value();

    /**
     * @brief is_valid checks whether the contained value is within the predefined bounds.
     * @return true iff min_value() <= value <= max_value().
     */
    inline bool is_valid() const
    {
        return (min_value() <= value) && (value <= max_value());
    }

    /**
     * @brief Current value.
     */
    int value;
};

/**
 * @brief Read the OomScoreAdj value for a process instance.
 * @throw std::runtime_error in case of errors.
 * @param [in] process The process to read the score for.
 * @param [out] score_adj The destination to store the value in.
 */
CORE_POSIX_DLL_PUBLIC const posix::Process& operator>>(const posix::Process& process, OomScoreAdj& score_adj);

/**
 * @brief Write the OomScoreAdj value for a process instance.
 * @throw std::runtime_error in case of errors and std::logic_error if score_adj.is_valid() returns false.
 * @param [in] process The process to write the score for.
 * @param [in] score_adj The new value to store.
 */
CORE_POSIX_DLL_PUBLIC const posix::Process& operator<<(const posix::Process& process,
                                 const OomScoreAdj& score_adj);
}
}
}
}
}
#endif // CORE_POSIX_LINUX_PROC_PROCESS_OOM_SCORE_ADJ_H_
