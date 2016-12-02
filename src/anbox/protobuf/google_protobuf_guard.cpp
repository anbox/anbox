/*
 * Copyright © 2012 Canonical Ltd.
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
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include <google/protobuf/stubs/common.h>

extern "C" int __attribute__((constructor)) init_google_protobuf() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  return 0;
}

extern "C" int __attribute__((destructor)) shutdown_google_protobuf() {
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}

// Preserve ABI
namespace anbox {
namespace protobuf {
void google_protobuf_guard();
}
}

void anbox::protobuf::google_protobuf_guard() {}
