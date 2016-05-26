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

#include <core/posix/linux/proc/process/oom_score_adj.h>

#include <core/posix/process.h>

#include <fstream>
#include <sstream>
#include <stdexcept>

#include <linux/oom.h>

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

int OomScoreAdj::min_value()
{
    return OOM_SCORE_ADJ_MIN;
}

int OomScoreAdj::max_value()
{
    return OOM_SCORE_ADJ_MAX;
}

const posix::Process& operator>>(const posix::Process& process, OomScoreAdj& score_adj)
{
    std::stringstream ss; ss << "/proc/" << process.pid() << "/oom_score_adj";
    std::ifstream in(ss.str());

    in >> score_adj.value;

    return process;
}

const posix::Process& operator<<(const posix::Process& process, const OomScoreAdj& score_adj)
{
    if (!score_adj.is_valid())
        throw std::logic_error("Value for adjusting the oom score is invalid.");

    std::stringstream ss; ss << "/proc/" << process.pid() << "/oom_score_adj";
    std::ofstream out(ss.str());

    out << score_adj.value;

    return process;
}
}
}
}
}
}
