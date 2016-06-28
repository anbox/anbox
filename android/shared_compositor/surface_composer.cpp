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

#include "surface_composer.h"

#define LOG_TAG "AnboxSurfaceComposer"
#include <cutils/log.h>

#include <ui/DisplayInfo.h>

#include <gui/DisplayEventReceiver.h>
#include <gui/IDisplayEventConnection.h>
#include <gui/BitTube.h>

#include <hardware/hwcomposer_defs.h>

namespace {
class DisplayDevice {
public:
    enum DisplayType {
        DISPLAY_ID_INVALID = -1,
        DISPLAY_PRIMARY     = HWC_DISPLAY_PRIMARY,
        DISPLAY_EXTERNAL    = HWC_DISPLAY_EXTERNAL,
        DISPLAY_VIRTUAL     = HWC_DISPLAY_VIRTUAL,
        NUM_BUILTIN_DISPLAY_TYPES = HWC_NUM_PHYSICAL_DISPLAY_TYPES,
    };
};

class DisplayEventConnection : public BnDisplayEventConnection {
public:
    DisplayEventConnection() :
        mChannel(new BitTube) {
    }

    status_t postEvent(const DisplayEventReceiver::Event& event) {
        return OK;
    }

private:
    virtual void setVsyncRate(uint32_t count) {
    }

    virtual void requestNextVsync() {
    }

    virtual void onFirstRef() {
    }

    virtual sp<BitTube> getDataChannel() const {
        return mChannel;
    }

private:
    sp<BitTube> const mChannel;
};

class Client : public BnSurfaceComposerClient {
public:
    Client() { }
    ~Client() { }

    status_t initCheck() const {
        return OK;
    }

private:
    virtual status_t createSurface(
            const String8& name,
            uint32_t w, uint32_t h,PixelFormat format, uint32_t flags,
            sp<IBinder>* handle,
            sp<IGraphicBufferProducer>* gbp) {
        return OK;
    }

    virtual status_t destroySurface(const sp<IBinder>& handle) {
        return OK;
    }

    virtual status_t clearLayerFrameStats(const sp<IBinder>& handle) const {
        return OK;
    }

    virtual status_t getLayerFrameStats(const sp<IBinder>& handle, FrameStats* outStats) const {
        return OK;
    }

    virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags) {
        return OK;
    }
};
}

namespace anbox {
namespace android {
SurfaceComposer::SurfaceComposer() :
    mPrimaryDisplay(new BBinder) {
}

void SurfaceComposer::binderDied(const wp<IBinder>&) {
    ALOGI("%s", __PRETTY_FUNCTION__);
}

sp<ISurfaceComposerClient> SurfaceComposer::createConnection() {
    ALOGI("%s", __PRETTY_FUNCTION__);
    sp<ISurfaceComposerClient> bclient;
    sp<Client> client(new Client);
    status_t err = client->initCheck();
    if (err == NO_ERROR) {
        bclient = client;
    }
    return bclient;
}

sp<IGraphicBufferAlloc> SurfaceComposer::createGraphicBufferAlloc() {
    ALOGI("%s", __PRETTY_FUNCTION__);
    return nullptr;
}

sp<IDisplayEventConnection> SurfaceComposer::createDisplayEventConnection() {
    ALOGI("%s", __PRETTY_FUNCTION__);
    return new DisplayEventConnection;
}

sp<IBinder> SurfaceComposer::createDisplay(const String8& displayName, bool secure) {
    ALOGI("%s", __PRETTY_FUNCTION__);
    return nullptr;
}

void SurfaceComposer::destroyDisplay(const sp<IBinder>& display) {
    ALOGI("%s", __PRETTY_FUNCTION__);
}

sp<IBinder> SurfaceComposer::getBuiltInDisplay(int32_t id) {
    if (id != DisplayDevice::DISPLAY_PRIMARY)
        return nullptr;

    return mPrimaryDisplay;
}

void SurfaceComposer::setTransactionState(const Vector<ComposerState>& state,
                         const Vector<DisplayState>& displays, uint32_t flags) {
    ALOGI("%s", __PRETTY_FUNCTION__);
}

void SurfaceComposer::bootFinished() {
    ALOGI("%s", __PRETTY_FUNCTION__);
}

bool SurfaceComposer::authenticateSurfaceTexture(const sp<IGraphicBufferProducer>& surface) const  {
    ALOGI("%s", __PRETTY_FUNCTION__);
    return true;
}

void SurfaceComposer::setPowerMode(const sp<IBinder>& display, int mode) {
    ALOGI("%s", __PRETTY_FUNCTION__);
}

status_t SurfaceComposer::getDisplayConfigs(const sp<IBinder>& display, Vector<DisplayInfo>* configs) {
    ALOGI("%s", __PRETTY_FUNCTION__);

    configs->clear();

    DisplayInfo info = DisplayInfo();
    info.w = 0;
    info.h = 0;
    info.xdpi = 0;
    info.ydpi = 0;
    info.fps = 60;
    info.secure = true;
    configs->push_back(info);

    return OK;
}

status_t SurfaceComposer::getDisplayStats(const sp<IBinder>& display, DisplayStatInfo* stats) {
    ALOGI("%s", __PRETTY_FUNCTION__);
    return OK;
}

int SurfaceComposer::getActiveConfig(const sp<IBinder>& display) {
    ALOGI("%s", __PRETTY_FUNCTION__);
    return 0;
}

status_t SurfaceComposer::setActiveConfig(const sp<IBinder>& display, int id) {
    ALOGI("%s", __PRETTY_FUNCTION__);
    return OK;
}

status_t SurfaceComposer::captureScreen(const sp<IBinder>& display,
        const sp<IGraphicBufferProducer>& producer,
        Rect sourceCrop, uint32_t reqWidth, uint32_t reqHeight,
        uint32_t minLayerZ, uint32_t maxLayerZ,
        bool useIdentityTransform,
        Rotation rotation) {
    ALOGI("%s", __PRETTY_FUNCTION__);
    return OK;
}

status_t SurfaceComposer::clearAnimationFrameStats() {
    ALOGI("%s", __PRETTY_FUNCTION__);
    return OK;
}

status_t SurfaceComposer::getAnimationFrameStats(FrameStats* outStats) const {
    ALOGI("%s", __PRETTY_FUNCTION__);
    return OK;
}
} // namespace android
} // namespace anbox
