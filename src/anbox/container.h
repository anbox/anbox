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

#ifndef ANBOX_CONTAINER_H_
#define ANBOX_CONTAINER_H_

#include <string>
#include <vector>
#include <map>
#include <memory.h>

#include "core/posix/child_process.h"

#include "anbox/do_not_copy_or_move.h"
#include "anbox/runtime.h"

#include <sys/types.h>

namespace anbox {
class Container : public DoNotCopyOrMove,
                  public std::enable_shared_from_this<Container> {
public:
    class Spec {
    public:
        static Spec Default();

        std::string rootfs_path;
        std::string init_command;
        std::vector<std::string> temporary_dirs;
        std::map<std::string,std::string> bind_paths;
        std::vector<std::string> dev_bind_paths;
        std::map<std::string,std::string> environment;
    };

    static std::shared_ptr<Container> create(const Spec &spec);

    virtual ~Container();

    void start();
    void stop();

private:
    Container(const Container::Spec &spec);

    Spec spec_;
    core::posix::ChildProcess child_;
    core::posix::ProcessGroup child_group_;
};
} // namespace anbox

#endif
