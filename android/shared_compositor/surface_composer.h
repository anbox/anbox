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

#ifndef ANBOX_ANDROID_SURFACE_COMPOSER_H_
#define ANBOX_ANDROID_SURFACE_COMPOSER_H_

#include <gui/ISurfaceComposer.h>
#include <ui/Rect.h>
#include <binder/BinderService.h>

using namespace android;

namespace anbox {
namespace android {
class SurfaceComposer : public BinderService<SurfaceComposer>,
                        public BnSurfaceComposer,
                        public IBinder::DeathRecipient {
public:
    static char const *getServiceName() { return "SurfaceFlinger"; }

    SurfaceComposer();

    void binderDied(const wp<IBinder>& who) override;

    sp<ISurfaceComposerClient> createConnection();
    sp<IGraphicBufferAlloc> createGraphicBufferAlloc();
    sp<IDisplayEventConnection> createDisplayEventConnection();
    sp<IBinder> createDisplay(const String8& displayName, bool secure);
    void destroyDisplay(const sp<IBinder>& display);
    sp<IBinder> getBuiltInDisplay(int32_t id);
    void setTransactionState(const Vector<ComposerState>& state,
                             const Vector<DisplayState>& displays, uint32_t flags);
    void bootFinished();
    bool authenticateSurfaceTexture(const sp<IGraphicBufferProducer>& surface) const;
    void setPowerMode(const sp<IBinder>& display, int mode);
    status_t getDisplayConfigs(const sp<IBinder>& display, Vector<DisplayInfo>* configs);
    status_t getDisplayStats(const sp<IBinder>& display, DisplayStatInfo* stats);
    int getActiveConfig(const sp<IBinder>& display);
    status_t setActiveConfig(const sp<IBinder>& display, int id);
    status_t captureScreen(const sp<IBinder>& display,
            const sp<IGraphicBufferProducer>& producer,
            Rect sourceCrop, uint32_t reqWidth, uint32_t reqHeight,
            uint32_t minLayerZ, uint32_t maxLayerZ,
            bool useIdentityTransform,
            Rotation rotation = eRotateNone);
    status_t clearAnimationFrameStats();
    status_t getAnimationFrameStats(FrameStats* outStats) const;

private:
    sp<IBinder> mPrimaryDisplay;
};
} // namespace android
} // namespace anbox

#endif
