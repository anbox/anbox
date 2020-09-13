/*
 * Copyright (C) 2020 Tomasz Grobelny <tomasz@grobelny.net>
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

#include "anbox/dbus/gps_server.h"

#include "anbox/android/intent.h"
#include "anbox/logger.h"

using namespace std;

void GpsServer::PushSentence(const string& sentence) {
  gps_info_broker_->newNmeaSentence(sentence);
}
