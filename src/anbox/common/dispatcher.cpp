/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
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
 *
 */

#include "anbox/common/dispatcher.h"

namespace {
struct AsioStrandDispatcher : public anbox::common::Dispatcher {
 public:
  AsioStrandDispatcher(const std::shared_ptr<anbox::Runtime>& rt)
      : rt{rt}, strand{rt->service()} {}

  void dispatch(const Task& task) override { strand.post(task); }

 private:
  std::shared_ptr<anbox::Runtime> rt;
  boost::asio::io_service::strand strand;
};
}

std::shared_ptr<anbox::common::Dispatcher>
anbox::common::create_dispatcher_for_runtime(
    const std::shared_ptr<anbox::Runtime>& rt) {
  return std::make_shared<AsioStrandDispatcher>(rt);
}
