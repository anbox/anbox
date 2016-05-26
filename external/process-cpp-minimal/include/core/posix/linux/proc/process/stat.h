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
#ifndef CORE_POSIX_LINUX_PROC_PROCESS_STAT_H_
#define CORE_POSIX_LINUX_PROC_PROCESS_STAT_H_

#include <core/posix/linux/proc/process/state.h>

#include <core/posix/visibility.h>

#include <string>

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
 * @brief The Stat struct encapsulates status information about a process.
 */
struct CORE_POSIX_DLL_PUBLIC Stat
{
    pid_t pid = 1; ///< The process ID
    std::string executable; ///< The filename of the executable, in parentheses.
    State state = State::undefined; ///< State of the process.
    pid_t parent = -1; ///< The PID of the parent.
    pid_t process_group = -1; ///< The process group ID of the process.
    int session_id = -1; ///< The session ID of the process.
    int tty_nr = -1; ///< The controlling terminal of the process.
    int controlling_process_group = -1; ///< The ID of the foreground process group of the controlling terminal of the process.
    unsigned int kernel_flags = 0; ///< The kernel flags word of the process.
    long unsigned int minor_faults_count = 0; ///< The number of minor faults the process has made which have not required loading a memory page from disk.
    long unsigned int minor_faults_count_by_children = 0; ///< The number of minor faults that the process's waited-for children have made.
    long unsigned int major_faults_count = 0; ///< The number of major faults the process has made which have required loading a memory page from disk.
    long unsigned int major_faults_count_by_children = 0; ///< The number of major faults that the process's waited-for children have made.
    struct
    {
        long unsigned int user = 0; ///<  Amount of time that this process has been scheduled in user mode, [clock ticks].
        long unsigned int system = 0; ///< Amount of time that this process has been scheduled in kernel mode, [clock ticks].
        long unsigned int user_for_children = 0; ///< Amount of time that this process's waited-for children have been scheduled in user mode, [clock ticks].
        long unsigned int system_for_children = 0; ///< Amount of time that this process's waited-for children have been scheduled in kernel mode, [clock ticks].
    } time;
    /**
     * (Explanation for Linux 2.6) For processes running a real-time scheduling
     * policy (policy below; see sched_setscheduler(2)), this is the negated
     * scheduling priority, minus one; that is, a number in the range -2 to
     * -100, corresponding to real-time priorities 1 to 99. For processes running
     * under a non-real-time scheduling policy, this is the raw nice value
     * (setpriority(2)) as represented in the kernel. The kernel stores nice
     * values as numbers in the range 0 (high) to 39 (low), corresponding to
     * the user-visible nice range of -20 to 19.
     *
     *Before Linux 2.6, this was a scaled value based on the scheduler
     *weighting given to this process.
     */
    long int priority = 0;
    long int nice = 0; ///< The nice value (see setpriority(2)), a value in the range 19 (low priority) to -20 (high priority).
    long int thread_count = 0; ///< Number of threads in this process (since Linux 2.6).
    long int time_before_next_sig_alarm = 0; ///< The time in jiffies before the next SIGALRM is sent to the process due to an interval timer. Since kernel 2.6.17, this field is no longer maintained, and  is hard coded as 0.
    long int start_time = 0; ///< The time the process started after system boot.  In kernels before Linux 2.6, this value was expressed in jiffies.  Since Linux 2.6, the value is expressed in clock ticks (divide by sysconf(_SC_CLK_TCK)).
    struct
    {
        long unsigned int virt = 0; ///< Virtual memory size in bytes.
        long unsigned int resident_set = 0; ///< Resident Set Size: number of pages the process has in real memory.  This is just the pages which count toward text, data, or stack space. This does not include pages which have not been demand-loaded in, or which are swapped out.
        long unsigned int resident_set_limit = 0; ///< Current soft limit in bytes on the rss of the process; see the description of RLIMIT_RSS in getrlimit(2).
    } size;
    struct
    {
        long unsigned int start_code = 0; ///< The address above which program text can run.
        long unsigned int end_code = 0; ///< The address below which program text can run.
        long unsigned int start_stack = 0; ///< The address of the start (i.e., bottom) of the stack.
        long unsigned int stack_pointer = 0; ///< The current value of ESP (stack pointer), as found in the kernel stack page for the process.
        long unsigned int instruction_pointer = 0; ///< The current EIP (instruction pointer).
    } addresses;
    struct
    {
        long unsigned int pending = 0; ///< The bitmap of pending signals, displayed as a decimal number.  Obsolete, because it does not provide information on real-time signals; use /proc/[pid]/status instead.
        long unsigned int blocked = 0; ///< The bitmap of blocked signals, displayed as a decimal number.  Obsolete, because it does not provide information on real-time signals; use  /proc/[pid]/status instead.
        long unsigned int ignored = 0; ///< The bitmap of ignored signals, displayed as a decimal number.  Obsolete, because it does not provide information on real-time signals; use /proc/[pid]/status instead.
        long unsigned int caught = 0; ///< The bitmap of caught signals, displayed as a decimal number.  Obsolete, because it does not provide information on real-time signals;  use  /proc/[pid]/status instead.
    } signals;
    long unsigned int channel = 0; ///< This is the "channel" in which the process is waiting.  It is the address of a system call, and can be looked up in a namelist if you need a textual name. (If you have an up-to-date /etc/psdatabase, then try ps -l to see the WCHAN field in action.)
    long unsigned int swap_count = 0; ///< Number of pages swapped (not maintained).
    long unsigned int swap_count_children = 0; ///< Cumulative nswap for child processes (not maintained).
    int exit_signal = -1; ///< Signal to be sent to parent when we die.
    int cpu_count = -1; ///< CPU number last executed on.
    unsigned int realtime_priority = 0; ///< Real-time scheduling priority, a number in the range 1 to 99 for processes scheduled under  a  real-time  policy,  or  0,  for  non-real-time  processes  (see sched_setscheduler(2)).
    unsigned int scheduling_policy = 0; ///< Scheduling policy (see sched_setscheduler(2)). Decode using the SCHED_* constants in linux/sched.h.
    long long unsigned int aggregated_block_io_delays = 0; ///< Aggregated block I/O delays, measured in clock ticks (centiseconds).
    long unsigned int guest_time = 0; ///< Guest time of the process (time spent running a virtual CPU for a guest operating system), measured in clock ticks.
    long unsigned int guest_time_children = 0; ///< Guest time of the process's children, measured in clock ticks.
};

CORE_POSIX_DLL_PUBLIC const posix::Process& operator>>(const posix::Process& process, Stat& stat);
}
}
}
}
}
#endif // CORE_POSIX_LINUX_PROC_PROCESS_STAT_H_
