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

#ifndef CORE_POSIX_SIGNAL_H_
#define CORE_POSIX_SIGNAL_H_

#include <core/posix/visibility.h>

#include <core/signal.h>

#include <signal.h>

#include <initializer_list>
#include <memory>

namespace core
{
namespace posix
{
/**
 * @brief The Signal enum collects the most common POSIX signals.
 */
enum class Signal
{
    unknown = 0,
    sig_hup = SIGHUP,
    sig_int = SIGINT,
    sig_quit = SIGQUIT,
    sig_ill = SIGILL,
    sig_abrt = SIGABRT,
    sig_fpe = SIGFPE,
    sig_kill = SIGKILL,
    sig_segv = SIGSEGV,
    sig_pipe = SIGPIPE,
    sig_alrm = SIGALRM,
    sig_term = SIGTERM,
    sig_usr1 = SIGUSR1,
    sig_usr2 = SIGUSR2,
    sig_chld = SIGCHLD,
    sig_cont = SIGCONT,
    sig_stop = SIGSTOP,
    sig_tstp = SIGTSTP,
    sig_ttin = SIGTTIN,
    sig_ttou = SIGTTOU
};

/**
 * @brief The SignalTrap class encapsulates functionality to trap and handle signals.
 */
class CORE_POSIX_DLL_PUBLIC SignalTrap
{
public:
    SignalTrap(const SignalTrap&) = delete;
    virtual ~SignalTrap() = default;

    SignalTrap& operator=(const SignalTrap&) = delete;
    bool operator==(const SignalTrap&) const = delete;

    /**
     * @brief Returns true if the given signal is trapped by this instance.
     */
    virtual bool has(Signal signal) = 0;

    /**
     * @brief Starts observation of incoming signals, relaying them via
     * signal_raised(). The call blocks until stop is called.
     */
    virtual void run() = 0;

    /**
     * @brief Stops execution of the signal trap.
     */
    virtual void stop() = 0;

    /**
     * @brief Emitted whenever a trapped signal is raised by the operating system.
     */
    virtual core::Signal<Signal>& signal_raised() = 0;

protected:
    SignalTrap() = default;
};

/**
  * @brief Traps the specified signals for the entire process.
  */
CORE_POSIX_DLL_PUBLIC
std::shared_ptr<SignalTrap> trap_signals_for_process(
        std::initializer_list<core::posix::Signal> blocked_signals);

/**
  * @brief Traps the specified signals for the current thread, and inherits
  * the respective signal mask to all child-threads.
  */
CORE_POSIX_DLL_PUBLIC
std::shared_ptr<SignalTrap> trap_signals_for_all_subsequent_threads(
        std::initializer_list<core::posix::Signal> blocked_signals);

}
}

#endif
