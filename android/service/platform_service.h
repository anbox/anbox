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

#ifndef ANBOX_ANDROID_PLATFORM_SERVICE_H_
#define ANBOX_ANDROID_PLATFORM_SERVICE_H_

#include "android/service/platform_service_interface.h"
#include "android/service/platform_api_stub.h"

#include <binder/Parcel.h>

#include <memory>

namespace android {
class PlatformService : public BnPlatformService {
public:
    static const char* service_name() { return "org.anbox.PlatformService"; }

    PlatformService(const std::shared_ptr<anbox::PlatformApiStub> &platform_api_stub);

    status_t boot_finished() override;
    status_t update_window_state(const Parcel &data) override;
    status_t update_application_list(const Parcel &data) override;
    status_t set_clipboard_data(const Parcel &data) override;
    status_t get_clipboard_data(const Parcel &data, Parcel *reply) override;

private:
    anbox::PlatformApiStub::WindowStateUpdate::Window unpack_window_state(const Parcel &data);
    std::shared_ptr<anbox::PlatformApiStub> platform_api_stub_;
};
} // namespace android

#endif
