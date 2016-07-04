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

#ifndef ANBOX_PLATFORM_API_SKELETON_H_
#define ANBOX_PLATFORM_API_SKELETON_H_

namespace google {
namespace protobuf {
class Closure;
} // namespace protobuf
} // namespace google

namespace core {
namespace posix {
class ChildProcess;
} // posix
} // core

namespace anbox {
namespace protobuf {
namespace bridge {
class InstallApplication;
class LaunchApplication;
class SetDnsServers;
class Void;
} // namespace bridge
} // namespace protobuf
class AndroidApiSkeleton {
public:
    AndroidApiSkeleton();
    ~AndroidApiSkeleton();

    void install_application(anbox::protobuf::bridge::InstallApplication const *request,
                             anbox::protobuf::bridge::Void *response,
                             google::protobuf::Closure *done);

    void launch_application(anbox::protobuf::bridge::LaunchApplication const *request,
                            anbox::protobuf::bridge::Void *response,
                            google::protobuf::Closure *done);

    void set_dns_servers(anbox::protobuf::bridge::SetDnsServers const *request,
                         anbox::protobuf::bridge::Void *response,
                         google::protobuf::Closure *done);

private:
    void wait_for_process(core::posix::ChildProcess &process,
                          anbox::protobuf::bridge::Void *response);
};
} // namespace anbox

#endif
