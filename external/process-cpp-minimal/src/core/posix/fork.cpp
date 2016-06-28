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

#include <core/posix/exit.h>
#include <core/posix/fork.h>

#ifndef ANDROID
#include "backtrace.h"
#endif

#include <iomanip>
#include <iostream>
#include <system_error>

#include <unistd.h>

namespace
{
void redirect_stream_to_fd(int fd, int stream)
{
    auto rc = ::dup2(fd, stream);
    if (rc == -1)
        throw std::system_error(errno, std::system_category());
}

void print_backtrace(std::ostream& out, const std::string& line_prefix)
{
#ifndef ANDROID
    core::posix::backtrace::visit_with_handler([&out, line_prefix](const core::posix::backtrace::Frame& frame)
    {
        out << line_prefix << std::dec << std::setw(2) << frame.depth() << "@" << std::hex << std::setw(14) << frame.frame_pointer() << ": "
            << (frame.symbol().is_cxx() ? frame.symbol().demangled() : frame.symbol().raw()) << std::endl;
        return true;
    });
#endif
}
}

namespace core
{
namespace posix
{

bool is_child(pid_t pid) { return pid == 0; }

ChildProcess fork(const std::function<posix::exit::Status()>& main,
                  const StandardStream& flags)
{
    ChildProcess::Pipe stdin_pipe{ChildProcess::Pipe::invalid()};
    ChildProcess::Pipe stdout_pipe{ChildProcess::Pipe::invalid()};
    ChildProcess::Pipe stderr_pipe{ChildProcess::Pipe::invalid()};

    if ((flags & StandardStream::stdin) != StandardStream::empty)
        stdin_pipe = ChildProcess::Pipe();
    if ((flags & StandardStream::stdout) != StandardStream::empty)
        stdout_pipe = ChildProcess::Pipe();
    if ((flags & StandardStream::stderr) != StandardStream::empty)
        stderr_pipe = ChildProcess::Pipe();

    pid_t pid = ::fork();

    if (pid == -1)
        throw std::system_error(errno, std::system_category());

    if (is_child(pid))
    {
        posix::exit::Status result = posix::exit::Status::failure;

        try
        {
            stdin_pipe.close_write_fd();
            stdout_pipe.close_read_fd();
            stderr_pipe.close_read_fd();
            // We replace stdin and stdout of the child process first:
            if ((flags & StandardStream::stdin) != StandardStream::empty)
                redirect_stream_to_fd(stdin_pipe.read_fd(), STDIN_FILENO);
            if ((flags & StandardStream::stdout) != StandardStream::empty)
                redirect_stream_to_fd(stdout_pipe.write_fd(), STDOUT_FILENO);
            if ((flags & StandardStream::stderr) != StandardStream::empty)
                redirect_stream_to_fd(stderr_pipe.write_fd(), STDERR_FILENO);

            result = main();
        } catch(const std::exception& e)
        {
            std::cerr << "core::posix::fork(): An unhandled std::exception occured in the child process:" << std::endl
                      << "  what(): " << e.what() << std::endl;
            print_backtrace(std::cerr, "  ");
        } catch(...)
        {
            std::cerr << "core::posix::fork(): An unhandled exception occured in the child process." << std::endl;
            print_backtrace(std::cerr, "  ");
        }

        // We have to ensure that we exit here. Otherwise, we run into
        // all sorts of weird issues.
        ::exit(static_cast<int>(result));
    }

    // We are in the parent process, and create a process object
    // corresponding to the newly forked process.
    stdin_pipe.close_read_fd();
    stdout_pipe.close_write_fd();
    stderr_pipe.close_write_fd();

    return ChildProcess(pid,
                        stdin_pipe,
                        stdout_pipe,
                        stderr_pipe);
}

ChildProcess vfork(const std::function<posix::exit::Status()>& main,
                   const StandardStream& flags)
{
    ChildProcess::Pipe stdin_pipe, stdout_pipe, stderr_pipe;

    pid_t pid = ::vfork();

    if (pid == -1)
        throw std::system_error(errno, std::system_category());

    if (is_child(pid))
    {
        posix::exit::Status result = posix::exit::Status::failure;

        try
        {
            // We replace stdin and stdout of the child process first:
            stdin_pipe.close_write_fd();
            stdout_pipe.close_read_fd();
            stderr_pipe.close_read_fd();
            // We replace stdin and stdout of the child process first:
            if ((flags & StandardStream::stdin) != StandardStream::empty)
                redirect_stream_to_fd(stdin_pipe.read_fd(), STDIN_FILENO);
            if ((flags & StandardStream::stdout) != StandardStream::empty)
                redirect_stream_to_fd(stdout_pipe.write_fd(), STDOUT_FILENO);
            if ((flags & StandardStream::stderr) != StandardStream::empty)
                redirect_stream_to_fd(stderr_pipe.write_fd(), STDERR_FILENO);

            result = main();
        } catch(const std::exception& e)
        {
            std::cerr << "core::posix::fork(): An unhandled std::exception occured in the child process:" << std::endl
                      << "  what(): " << e.what() << std::endl;
            print_backtrace(std::cerr, "  ");
        } catch(...)
        {
            std::cerr << "core::posix::fork(): An unhandled exception occured in the child process." << std::endl;
            print_backtrace(std::cerr, "  ");
        }

        // We have to ensure that we exit here. Otherwise, we run into
        // all sorts of weird issues.
        ::exit(static_cast<int>(result));
    }

    // We are in the parent process, and create a process object
    // corresponding to the newly forked process.
    // Close the parent's pipe end
    stdin_pipe.close_read_fd();
    stdout_pipe.close_write_fd();
    stderr_pipe.close_write_fd();

    return ChildProcess(pid,
                        stdin_pipe,
                        stdout_pipe,
                        stderr_pipe);
}
}
}
