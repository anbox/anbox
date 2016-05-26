/*
 * Copyright © 2013 Canonical Ltd.
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

#include <core/posix/linux/proc/process/stat.h>

#include <core/posix/process.h>

#include <fstream>
#include <istream>
#include <sstream>

namespace core
{
namespace posix
{
namespace linux
{
namespace proc
{
namespace process
{
std::istream& operator>>(std::istream& in, State& state)
{
    char c; in >> c; state = static_cast<State>(c);
    return in;
}

std::istream& operator>>(std::istream& in, Stat& stat)
{
    in >> stat.pid
       >> stat.executable
       >> stat.state
       >> stat.parent
       >> stat.process_group
       >> stat.session_id
       >> stat.tty_nr
       >> stat.controlling_process_group
       >> stat.kernel_flags
       >> stat.minor_faults_count
       >> stat.minor_faults_count_by_children
       >> stat.major_faults_count
       >> stat.major_faults_count_by_children
       >> stat.time.user
       >> stat.time.system
       >> stat.time.user_for_children
       >> stat.time.system_for_children
       >> stat.priority
       >> stat.nice
       >> stat.thread_count
       >> stat.time_before_next_sig_alarm
       >> stat.start_time
       >> stat.size.virt
       >> stat.size.resident_set
       >> stat.size.resident_set_limit
       >> stat.addresses.start_code
       >> stat.addresses.end_code
       >> stat.addresses.start_stack
       >> stat.addresses.stack_pointer
       >> stat.addresses.instruction_pointer
       >> stat.signals.pending
       >> stat.signals.blocked
       >> stat.signals.ignored
       >> stat.signals.caught
       >> stat.channel
       >> stat.swap_count
       >> stat.swap_count_children
       >> stat.exit_signal
       >> stat.cpu_count
       >> stat.realtime_priority
       >> stat.scheduling_policy
       >> stat.aggregated_block_io_delays
       >> stat.guest_time
       >> stat.guest_time_children;

    return in;
}

const posix::Process& operator>>(const posix::Process& process, Stat& stat)
{
    std::stringstream ss; ss << "/proc/" << process.pid() << "/stat";
    std::ifstream in(ss.str());

    in >> stat;

    return process;
}
}
}
}
}
}
