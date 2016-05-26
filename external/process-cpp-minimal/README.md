process-cpp         {#mainpage}
===========

process-cpp is a simple and straightforward wrapper around process creation and
control targeted towards linux. It helps both with handling child processes and with
interacting with the current process. Some of its features include:

 - Thread-safe get/set/unset operation on the current process's environment.
 - Throwing and non-throwing overloads of functions when system calls are involved.
 - Seamless redirection of input, output and error streams of child processes.
 - Type-safe interaction with the virtual proc filesystem, both for reading & writing.

The library's main purpose is to assist in testing and when a software component
needs to carry out process creation/control tasks, e.g., a graphical shell. To this end,
the library is extensively tested and tries to ensure fail-safe operation as much as possible.

A simple echo
-------------

~~~~~~~~~~~~~{.cpp}
// Fork and run a simple echo:
posix::ChildProcess child = posix::fork(
            []()
            {
                std::string line;
                while(true)
                {
                    std::cin >> line;
                    std::cout << line << std::endl;
                }
                return EXIT_FAILURE;
            },
            posix::StandardStreamFlags()
                .set(posix::StandardStream::stdin)
                .set(posix::StandardStream::stdout));

// Check that the resulting process has a valid pid.
EXPECT_TRUE(child.pid() > 0);

// Check on echo functionality.
const std::string echo_value{"42"};
child.cin() << echo_value << std::endl;
std::string line; child.cout() >> line;
EXPECT_EQ(echo_value, line);

// Stop the process and synchronize with the process changing state.
EXPECT_NO_THROW(child.send_signal(posix::Signal::sig_stop));
auto result = child.wait_for(posix::wait::Flag::untraced);
EXPECT_EQ(posix::wait::Result::Status::stopped,
          result.status);
EXPECT_EQ(posix::Signal::sig_stop,
          result.detail.if_stopped.signal);

// Kill the stopped process and synchronize to its state change.
EXPECT_NO_THROW(child.send_signal(posix::Signal::sig_kill));
result = child.wait_for(posix::wait::Flag::untraced);
EXPECT_EQ(posix::wait::Result::Status::signaled,
          result.status);
EXPECT_EQ(posix::Signal::sig_kill,
          result.detail.if_signaled.signal);
~~~~~~~~~~~~~

Adjusting OOM Score Values
--------------------------

~~~~~~~~~~~~~{.cpp}
// Setup the manipulator with a well-known value.
posix::linux::proc::process::OomScoreAdj oom_score_adj
{
    posix::linux::proc::process::OomScoreAdj::max_value()
};
// Apply the manipulator to the current process
EXPECT_NO_THROW(posix::this_process::instance() << oom_score_adj);
// Read back the manipulators value for the current process
EXPECT_NO_THROW(posix::this_process::instance() >> oom_score_adj);
// And check that applying the manipulator was successful.
EXPECT_EQ(posix::linux::proc::process::OomScoreAdj::max_value(),
          oom_score_adj.value);
// Instantiate the observer...
posix::linux::proc::process::OomScore oom_score;
// ... and fill in its value for the current process.
EXPECT_NO_THROW(posix::this_process::instance() >> oom_score);
// Check that applying the manipulator before results in adjustments to the
// OOM score.
EXPECT_TRUE(is_approximately_equal(oom_score.value, posix::linux::proc::process::OomScoreAdj::max_value()));
~~~~~~~~~~~~~
