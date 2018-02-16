/*
 * Copyright (C) 2016 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <iostream>

#include "anbox/logger.h"
#include "anbox/runtime.h"

namespace {
// exception_safe_run runs service, catching all exceptions and
// restarting operation until an explicit shutdown has been requested.
//
// TODO(tvoss): Catching all exceptions is risky as they might signal
// unrecoverable
// errors. We should enable calling code to decide whether an exception should
// be considered
// fatal or not.
void exception_safe_run(boost::asio::io_service& service) {
  while (true) {
    try {
      service.run();
      // a clean return from run only happens in case of
      // stop() being called (we are keeping the service alive with
      // a service::work instance).
      break;
    } catch (const std::exception& e) {
      ERROR("%s", e.what());
    } catch (...) {
      ERROR("Unknown exception caught while executing boost::asio::io_service");
    }
  }
}
}
namespace anbox {

std::shared_ptr<Runtime> Runtime::create(std::uint32_t pool_size) {
  return std::shared_ptr<Runtime>(new Runtime(pool_size));
}

Runtime::Runtime(std::uint32_t pool_size)
    : pool_size_{pool_size},
      
      #if BOOST_VERSION >= 106600
      service_{static_cast<int>(pool_size_)},
      #else
      service_{pool_size_},
      #endif
      
      strand_{service_},
      keep_alive_{service_} {}

Runtime::~Runtime() noexcept(true) {
  try {
    stop();
  } catch (...) {
    // Dropping all exceptions to satisfy the nothrow guarantee.
  }
}

void Runtime::start() {
  for (unsigned int i = 0; i < pool_size_; i++)
    workers_.push_back(std::thread{exception_safe_run, std::ref(service_)});
}

void Runtime::stop() {
  service_.stop();

  for (auto& worker : workers_)
    if (worker.joinable())
      worker.join();
}

std::function<void(std::function<void()>)> Runtime::to_dispatcher_functional() {
  // We have to make sure that we stay alive for as long as
  // calling code requires the dispatcher to work.
  auto sp = shared_from_this();
  return [sp](std::function<void()> task) { sp->strand_.post(task); };
}

boost::asio::io_service& Runtime::service() { return service_; }

}  // namespace anbox
