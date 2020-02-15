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

#include <core/posix/signalable.h>

#include <poll.h>
#include <pthread.h>
#include <sys/eventfd.h>
#include <sys/signalfd.h>

#include <unistd.h>

#include <atomic>

namespace impl
{
void set_thread_signal_mask(::sigset_t* new_mask, ::sigset_t* old_mask)
{
    ::pthread_sigmask(SIG_BLOCK, new_mask, old_mask);
}

void set_process_signal_mask(::sigset_t* new_mask, ::sigset_t* old_mask)
{
    ::sigprocmask(SIG_BLOCK, new_mask, old_mask);
}

class SignalTrap : public core::posix::SignalTrap
{
public:
    enum class Scope
    {
        process,
        thread
    };

    enum class State
    {
        not_running,
        running
    };

    SignalTrap(Scope scope, std::initializer_list<core::posix::Signal> blocked_signals)
        : scope(scope),
          state(State::not_running),
          event_fd(::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK))
    {
        if (event_fd == -1)
            throw std::system_error(errno, std::system_category());

        ::sigemptyset(&blocked_signals_mask);

        for(auto signal : blocked_signals)
            ::sigaddset(&blocked_signals_mask, static_cast<int>(signal));

        switch (scope)
        {
        case Scope::process:
            set_process_signal_mask(&blocked_signals_mask, &old_signals_mask);
            break;
        case Scope::thread:
            set_thread_signal_mask(&blocked_signals_mask, &old_signals_mask);
            break;
        default:
            break;
        }
    }

    ~SignalTrap()
    {
        switch (scope)
        {
        case Scope::process:
            set_process_signal_mask(&old_signals_mask, nullptr);
            break;
        case Scope::thread:
            set_thread_signal_mask(&old_signals_mask, nullptr);
            break;
        default:
            break;
        }

        ::close(event_fd);
    }

    bool has(core::posix::Signal signal) override
    {
        return ::sigismember(&blocked_signals_mask, static_cast<int>(signal));
    }

    void run() override
    {
        static constexpr int signal_fd_idx = 0;
        static constexpr int event_fd_idx = 1;

        static constexpr int signal_info_buffer_size = 5;

        if (state.load() == State::running)
            throw std::runtime_error("SignalTrap::run can only be run once.");

        state.store(State::running);

        // Make sure we clean up the signal fd whenever
        // we leave the scope of run.
        struct Scope
        {
            ~Scope()
            {
                if (signal_fd != -1)
                    ::close(signal_fd);
            }

            int signal_fd;
        } scope{::signalfd(-1, &blocked_signals_mask, SFD_CLOEXEC | SFD_NONBLOCK)};

        if (scope.signal_fd == -1)
            throw std::system_error(errno, std::system_category());

        pollfd fds[2];
        signalfd_siginfo signal_info[signal_info_buffer_size];

        for (;;)
        {
            fds[signal_fd_idx] = {scope.signal_fd, POLLIN, 0};
            fds[event_fd_idx] = {event_fd, POLLIN, 0};

            auto rc = ::poll(fds, 2, -1);

            if (rc == -1)
            {
                if (errno == EINTR)
                    continue;

                break;
            }

            if (rc == 0)
                continue;

            if (fds[signal_fd_idx].revents & POLLIN)
            {
                auto result = ::read(scope.signal_fd, signal_info, sizeof(signal_info));

                for (unsigned int i = 0; i < result / sizeof(signalfd_siginfo); i++)
                {
                    if (has(static_cast<core::posix::Signal>(signal_info[i].ssi_signo)))
                    {
                        on_signal_raised(
                                    static_cast<core::posix::Signal>(
                                        signal_info[i].ssi_signo));
                    }
                }
            }

            if (fds[event_fd_idx].revents & POLLIN)
            {
                std::int64_t value{1};
                // Consciously void-ing the return value here.
                // Not much we can do about an error.
                auto result = ::read(event_fd, &value, sizeof(value));
                (void) result;

                break;
            }
        }

        state.store(State::not_running);
    }

    void stop() override
    {
        static const std::int64_t value = {1};
        if (sizeof(value) != ::write(event_fd, &value, sizeof(value)))
            throw std::system_error(errno, std::system_category());
    }

    core::Signal<core::posix::Signal>& signal_raised() override
    {
        return on_signal_raised;
    }

private:
    Scope scope;
    std::atomic<State> state;
    int event_fd;
    core::Signal<core::posix::Signal> on_signal_raised;
    ::sigset_t old_signals_mask;
    ::sigset_t blocked_signals_mask;
};
}

std::shared_ptr<core::posix::SignalTrap> core::posix::trap_signals_for_process(
	std::initializer_list<core::posix::Signal> blocked_signals)
{
    return std::make_shared<impl::SignalTrap>(
                impl::SignalTrap::Scope::process,
                blocked_signals);
}

std::shared_ptr<core::posix::SignalTrap> core::posix::trap_signals_for_all_subsequent_threads(
        std::initializer_list<core::posix::Signal> blocked_signals)
{
    return std::make_shared<impl::SignalTrap>(
                impl::SignalTrap::Scope::thread,
                blocked_signals);
}

