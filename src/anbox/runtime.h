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

#ifndef ANBOX_RUNTIME_H_
#define ANBOX_RUNTIME_H_

#include <boost/asio.hpp>
#include <boost/version.hpp>

#include <memory.h>
#include <functional>
#include <thread>

#include "anbox/do_not_copy_or_move.h"

namespace anbox {

// We bundle our "global" runtime dependencies here, specifically
// a dispatcher to decouple multiple in-process providers from one
// another , forcing execution to a well known set of threads.
class Runtime : public DoNotCopyOrMove,
                public std::enable_shared_from_this<Runtime> {
 public:
  // Our default concurrency setup.
  static constexpr const std::uint32_t worker_threads = 8;

  // create returns a Runtime instance with pool_size worker threads
  // executing the underlying service.
  static std::shared_ptr<Runtime> create(
      std::uint32_t pool_size = worker_threads);

  // Tears down the runtime, stopping all worker threads.
  ~Runtime() noexcept(true);

  // start executes the underlying io_service on a thread pool with
  // the size configured at creation time.
  void start();

  // stop cleanly shuts down a Runtime instance.
  void stop();

  // to_dispatcher_functional returns a function for integration
  // with components that expect a dispatcher for operation.
  std::function<void(std::function<void()>)> to_dispatcher_functional();

  // service returns the underlying boost::asio::io_service that is executed
  // by the Runtime.
  boost::asio::io_service& service();

 private:
  // Runtime constructs a new instance, firing up pool_size
  // worker threads.
  Runtime(std::uint32_t pool_size);

  std::uint32_t pool_size_;
  boost::asio::io_service service_;
  boost::asio::io_service::strand strand_;
  boost::asio::io_service::work keep_alive_;
  std::vector<std::thread> workers_;
};

}  // namespace anbox

#endif
