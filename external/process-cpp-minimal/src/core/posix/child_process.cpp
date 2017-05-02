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

#include <core/posix/child_process.h>

#ifndef ANDROID
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#endif

#include <atomic>
#include <fstream>
#include <mutex>
#include <unordered_map>

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <sys/eventfd.h>
#include <sys/signalfd.h>

#include <assert.h>

#include <sys/wait.h>

#ifndef ANDROID
namespace io = boost::iostreams;
#endif

namespace
{

struct DeathObserverImpl : public core::posix::ChildProcess::DeathObserver
{
    DeathObserverImpl(const std::shared_ptr<core::posix::SignalTrap>& trap)
        : on_sig_child_connection
          {
              trap->signal_raised().connect([this](core::posix::Signal signal)
              {
                  switch (signal)
                  {
                  case core::posix::Signal::sig_chld:
                    on_sig_child();
                    break;
                  default:
                    break;
                  }
              })
          }
    {
        if (!trap->has(core::posix::Signal::sig_chld))
            throw std::logic_error(
                    "DeathObserver::DeathObserverImpl: Given SignalTrap"
                    " instance does not trap Signal::sig_chld.");
    }

    bool add(const core::posix::ChildProcess& process) override
    {
        if (process.pid() == -1)
            return false;

        std::lock_guard<std::mutex> lg(guard);

        bool added = false;
        auto new_process = std::make_pair(process.pid(), process);
        std::tie(std::ignore, added) = children.insert(new_process);

        if (added)
        {
            // The process may have died between it's instantiation and it
            // being added to the children map. Check that it's still alive.
            int status{-1};
            if (::waitpid(process.pid(), &status, WNOHANG) != 0) // child no longer alive
            {
                // we missed the SIGCHLD signal so we must now manually
                // inform our subscribers.
                signals.child_died(new_process.second);
                children.erase(new_process.first);
                return false;
            }
        }

        return added;
    }

    bool has(const core::posix::ChildProcess& process) const override
    {
        std::lock_guard<std::mutex> lg(guard);
        return children.count(process.pid()) > 0;
    }

    const core::Signal<core::posix::ChildProcess>& child_died() const override
    {
        return signals.child_died;
    }

    void on_sig_child() override
    {
        pid_t pid{-1}; int status{-1};
        while (true)
        {
            pid = ::waitpid(0, &status, WNOHANG);

            if (pid == -1)
            {
                if (errno == ECHILD)
                {
                    break; // No more children
                }
                continue; // Ignore stray SIGCHLD signals
            }
            else if (pid == 0)
            {
                break; // No more children
            }
            else
            {
                std::lock_guard<std::mutex> lg(guard);
                auto it = children.find(pid);

                if (it != children.end())
                {
                    if (WIFSIGNALED(status) || WIFEXITED(status))
                    {
                        signals.child_died(it->second);
                        children.erase(it);
                    }
                }
            }
        }
    }

    mutable std::mutex guard;
    std::unordered_map<pid_t, core::posix::ChildProcess> children;
    core::ScopedConnection on_sig_child_connection;
    struct
    {
        core::Signal<core::posix::ChildProcess> child_died;
    } signals;
};
}

std::unique_ptr<core::posix::ChildProcess::DeathObserver>
core::posix::ChildProcess::DeathObserver::create_once_with_signal_trap(
        std::shared_ptr<core::posix::SignalTrap> trap)
{
    static std::atomic<bool> has_been_created_once{false};

    if (has_been_created_once.exchange(true))
        throw std::runtime_error
        {
            "DeathObserver::create_once_with_signal_trap: "
            "Cannot create more than one instance."
        };

    try
    {
        std::unique_ptr<core::posix::ChildProcess::DeathObserver> result
        {
            new DeathObserverImpl{trap}
        };

        return result;
    } catch(...)
    {
        // We make sure that a throwing c'tor does not impact our ability to
        // retry creation of a DeathObserver instance.
        has_been_created_once.store(false);

        std::rethrow_exception(std::current_exception());
    }

    assert(false && "We should never reach here.");

    // Silence the compiler.
    return std::unique_ptr<core::posix::ChildProcess::DeathObserver>{};
}

namespace core
{
namespace posix
{
ChildProcess::Pipe ChildProcess::Pipe::invalid()
{
    static Pipe p;
    static std::once_flag flag;

    std::call_once(flag, [&]() { p.close_read_fd(); p.close_write_fd(); });

    return p;
}

ChildProcess::Pipe::Pipe()
{
    int rc = ::pipe(fds);

    if (rc == -1)
        throw std::system_error(errno, std::system_category());
}

ChildProcess::Pipe::Pipe(const ChildProcess::Pipe& rhs) : fds{-1, -1}
{
    if (rhs.fds[0] != -1)
        fds[0] = ::dup(rhs.fds[0]);

    if (rhs.fds[1] != -1)
        fds[1] = ::dup(rhs.fds[1]);
}

ChildProcess::Pipe::~Pipe()
{
    if (fds[0] != -1)
        ::close(fds[0]);
    if (fds[1] != -1)
        ::close(fds[1]);
}

int ChildProcess::Pipe::read_fd() const
{
    return fds[0];
}

void ChildProcess::Pipe::close_read_fd()
{
    if (fds[0] != -1)
    {
        ::close(fds[0]);
        fds[0] = -1;
    }
}

int ChildProcess::Pipe::write_fd() const
{
    return fds[1];
}

void ChildProcess::Pipe::close_write_fd()
{
    if (fds[1] != -1)
    {
        ::close(fds[1]);
        fds[1] = -1;
    }
}

ChildProcess::Pipe& ChildProcess::Pipe::operator=(const ChildProcess::Pipe& rhs)
{
    if (fds[0] != -1)
        ::close(fds[0]);
    if (fds[1] != -1)
        ::close(fds[1]);

    if (rhs.fds[0] != -1)
        fds[0] = ::dup(rhs.fds[0]);
    else
        fds[0] = -1;
    if (rhs.fds[1] != -1)
        fds[1] = ::dup(rhs.fds[1]);
    else
        fds[1] = -1;

    return *this;
}

struct ChildProcess::Private
{
    // stdin and stdout are always "relative" to the childprocess, i.e., we
    // write to stdin of the child process and read from its stdout.
    Private(pid_t pid,
            const ChildProcess::Pipe& stderr,
            const ChildProcess::Pipe& stdin,
            const ChildProcess::Pipe& stdout)
        : pipes{stderr, stdin, stdout},
#ifndef ANDROID
          serr(pipes.stderr.read_fd(), io::never_close_handle),
          sin(pipes.stdin.write_fd(), io::never_close_handle),
          sout(pipes.stdout.read_fd(), io::never_close_handle),
          cerr(&serr),
          cin(&sin),
          cout(&sout),
#endif
          original_parent_pid(::getpid()),
          original_child_pid(pid)
    {
    }

    ~Private()
    {
        // Check if we are in the original parent process.
        if (original_parent_pid == getpid() && !dont_kill_on_cleanup)
        {
            // If so, check if we are considering a valid pid here.
            // If so, we kill the original child.
            if (original_child_pid != -1)
                ::kill(original_child_pid, SIGKILL);
        }
    }

    struct
    {
        ChildProcess::Pipe stdin;
        ChildProcess::Pipe stdout;
        ChildProcess::Pipe stderr;
    } pipes;

#ifndef ANDROID
    io::stream_buffer<io::file_descriptor_source> serr;
    io::stream_buffer<io::file_descriptor_sink> sin;
    io::stream_buffer<io::file_descriptor_source> sout;
    std::istream cerr;
    std::ostream cin;
    std::istream cout;
#endif

    // We need to store the original parent pid as we might have been forked
    // and with our automatic cleanup in place, it might happen that the d'tor
    // is called from the child process.
    pid_t original_parent_pid;
    pid_t original_child_pid;

    bool dont_kill_on_cleanup = false;
};

ChildProcess ChildProcess::invalid()
{
    // We take the init process as child.
    static const pid_t invalid_pid = 1;
    return ChildProcess(invalid_pid, Pipe::invalid(), Pipe::invalid(), Pipe::invalid());
}

ChildProcess::ChildProcess(pid_t pid,
                           const ChildProcess::Pipe& stdin_pipe,
                           const ChildProcess::Pipe& stdout_pipe,
                           const ChildProcess::Pipe& stderr_pipe)
    : Process(pid),
      d(new Private{pid, stdin_pipe, stdout_pipe, stderr_pipe})
{
}

ChildProcess::~ChildProcess()
{
}

wait::Result ChildProcess::wait_for(const wait::Flags& flags)
{
    int status = -1;
    pid_t result_pid = ::waitpid(pid(), std::addressof(status), static_cast<int>(flags));

    if (result_pid == -1)
        throw std::system_error(errno, std::system_category());

    wait::Result result;

    if (result_pid == 0)
    {
        result.status = wait::Result::Status::no_state_change;
        return result;
    }

    if (WIFEXITED(status))
    {
        result.status = wait::Result::Status::exited;
        result.detail.if_exited.status = static_cast<exit::Status>(WEXITSTATUS(status));
    } else if (WIFSIGNALED(status))
    {
        result.status = wait::Result::Status::signaled;
        result.detail.if_signaled.signal = static_cast<Signal>(WTERMSIG(status));
        result.detail.if_signaled.core_dumped = WCOREDUMP(status);
    } else if (WIFSTOPPED(status))
    {
        result.status = wait::Result::Status::stopped;
        result.detail.if_stopped.signal = static_cast<Signal>(WSTOPSIG(status));
    }
#ifndef ANDROID
    else if (WIFCONTINUED(status))
    {
        result.status = wait::Result::Status::continued;
    }
#endif

    return result;
}

void ChildProcess::dont_kill_on_cleanup()
{
  d->dont_kill_on_cleanup = true;
}

#ifndef ANDROID
std::istream& ChildProcess::cerr()
{
    return d->cerr;
}

std::ostream& ChildProcess::cin()
{
    return d->cin;
}

std::istream& ChildProcess::cout()
{
    return d->cout;
}
#endif
}
}
