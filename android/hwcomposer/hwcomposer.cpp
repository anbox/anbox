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

#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>

#include <map>
#include <vector>
#include <algorithm>
#include <string>

#define LOG_NDEBUG 1
#include <cutils/log.h>

#include "HostConnection.h"
#include "gralloc_cb.h"

#define DEFINE_HOST_CONNECTION() \
    HostConnection *hostCon = HostConnection::get(); \
    renderControl_encoder_context_t *rcEnc = (hostCon ? hostCon->rcEncoder() : NULL)

#define DEFINE_AND_VALIDATE_HOST_CONNECTION() \
    HostConnection *hostCon = HostConnection::get(); \
    if (!hostCon) { \
        ALOGE("hwcomposer.anbox: Failed to get host connection\n"); \
        return -EIO; \
    } \
    renderControl_encoder_context_t *rcEnc = hostCon->rcEncoder(); \
    if (!rcEnc) { \
        ALOGE("hwcomposer.anbox: Failed to get renderControl encoder context\n"); \
        return -EIO; \
    }

struct HwcContext {
    hwc_composer_device_1_t device;

    // These 3 variables could be reduced to first_overlay only, however it makes
    // the conditions in the code more complicated. In order to keep things as
    // simple as possible, there are 3 major ways to display a frame.
    // 1. Show only the framebuffer.
    // 2. Show the framebuffer with some overlays above it.
    // 3. Show all overlays and hide the framebuffer.
    //
    // Since the framebuffer has no alpha channel and is opaque, it can only ever
    // be the rearmost layer that we end up putting on screen, otherwise it will
    // cover up all layers behind it, since its display frame is the whole window.
    //
    // Without framebuffer_visible, the condition of whether to display the
    // frambuffer becomes more complex and possibly if (numHwLayers == 0 ||
    // hwLayers[0]->compositionType != HWC_OVERLAY) but that might not be correct.
    //
    // The range [first_overlay, first_overlay+num_overlay) is a natural way to
    // structure the loop and prevents requiring state and iterating through all
    // the non-OVERLAY layers in hwc_set.
    bool framebuffer_visible;
    size_t first_overlay;
    size_t num_overlays;
};

static void dump_layer(hwc_layer_1_t const* l) {
    ALOGD("\tname='%s', type=%d, flags=%08x, handle=%p, tr=%02x, blend=%04x, {%d,%d,%d,%d}, {%d,%d,%d,%d}",
            l->name, l->compositionType, l->flags, l->handle, l->transform, l->blending,
            l->sourceCrop.left,
            l->sourceCrop.top,
            l->sourceCrop.right,
            l->sourceCrop.bottom,
            l->displayFrame.left,
            l->displayFrame.top,
            l->displayFrame.right,
            l->displayFrame.bottom);
}

static int hwc_prepare(hwc_composer_device_1_t* dev, size_t numDisplays,
                       hwc_display_contents_1_t** displays) {
    auto context = reinterpret_cast<HwcContext*>(dev);

    if (displays == NULL || displays[0] == NULL)
        return -EINVAL;

    // Anbox only supports the primary display.
    if (displays[0]->flags & HWC_GEOMETRY_CHANGED) {
        const size_t& num_hw_layers = displays[0]->numHwLayers;
        size_t i = 1;
        bool visible = (num_hw_layers == 1);

        // Iterate backwards and skip the first (end) layer, which is the
        // framebuffer target layer. According to the SurfaceFlinger folks, the
        // actual location of this layer is up to the HWC implementation to
        // decide, but is in the well know last slot of the list. This does not
        // imply that the framebuffer target layer must be topmost.
        for (; i < num_hw_layers; i++) {
          hwc_layer_1_t* layer = &displays[0]->hwLayers[num_hw_layers - 1 - i];

#if 0
          dump_layer(layer);
#endif

          if (layer->flags & HWC_SKIP_LAYER) {
            // All layers below and including this one will be drawn into the
            // framebuffer. Stop marking further layers as HWC_OVERLAY.
            visible = true;
            break;
          }

          switch (layer->compositionType) {
            case HWC_OVERLAY:
            case HWC_FRAMEBUFFER:
              layer->compositionType = HWC_OVERLAY;
              break;
            case HWC_BACKGROUND:
              break;
            default:
              ALOGE("hwcomposor: Invalid compositionType %d",
                      layer->compositionType);
              break;
          }
        }
        context->first_overlay = num_hw_layers - i;
        context->num_overlays = i - 1;
        context->framebuffer_visible = visible;
    }

    return 0;
}

/*
 * We're using "implicit" synchronization, so make sure we aren't passing any
 * sync object descriptors around.
 */
static void check_sync_fds(size_t numDisplays, hwc_display_contents_1_t** displays)
{
    unsigned int i, j;
    for (i = 0; i < numDisplays; i++) {
        hwc_display_contents_1_t* list = displays[i];
        if (list->retireFenceFd >= 0) {
            ALOGW("retireFenceFd[%u] was %d", i, list->retireFenceFd);
            list->retireFenceFd = -1;
        }

        for (j = 0; j < list->numHwLayers; j++) {
            hwc_layer_1_t* layer = &list->hwLayers[j];
            if (layer->acquireFenceFd >= 0) {
                ALOGW("acquireFenceFd[%u][%u] was %d, closing", i, j, layer->acquireFenceFd);
                close(layer->acquireFenceFd);
                layer->acquireFenceFd = -1;
            }
            if (layer->releaseFenceFd >= 0) {
                ALOGW("releaseFenceFd[%u][%u] was %d", i, j, layer->releaseFenceFd);
                layer->releaseFenceFd = -1;
            }
        }
    }
}

static int hwc_set(hwc_composer_device_1_t* dev, size_t numDisplays,
                   hwc_display_contents_1_t** displays) {
    auto context = reinterpret_cast<HwcContext*>(dev);

    if (displays == NULL || displays[0] == NULL)
        return -EFAULT;

    DEFINE_AND_VALIDATE_HOST_CONNECTION();

    for (size_t i = 0 ; i < displays[0]->numHwLayers ; i++) {
        const auto layer = &displays[0]->hwLayers[i];

        if (layer->flags & HWC_SKIP_LAYER ||
            layer->flags & HWC_IS_CURSOR_LAYER)
            continue;

#if 0
        dump_layer(layer);
#endif

        // FIXME this is just dirty ... but layer->handle is a const native_handle_t and canBePosted
        // can't be called with a const.
        auto cb = const_cast<cb_handle_t*>(reinterpret_cast<const cb_handle_t*>(layer->handle));
        if (!cb_handle_t::validate(cb)) {
            ALOGE("Buffer handle is invalid\n");
            return -EINVAL;
        }

        rcEnc->rcPostLayer(rcEnc,
                           layer->name,
                           cb->hostHandle,
                           layer->planeAlpha / 255,
                           layer->sourceCrop.left,
                           layer->sourceCrop.top,
                           layer->sourceCrop.right,
                           layer->sourceCrop.bottom,
                           layer->displayFrame.left,
                           layer->displayFrame.top,
                           layer->displayFrame.right,
                           layer->displayFrame.bottom);
        hostCon->flush();
    }

    rcEnc->rcPostAllLayersDone(rcEnc);

    check_sync_fds(numDisplays, displays);

    return 0;
}

static int hwc_event_control(hwc_composer_device_1* dev, int disp,
                             int event, int enabled) {
    return -EFAULT;
}

static void hwc_register_procs(hwc_composer_device_1* dev,
                               hwc_procs_t const* procs) {
}

static int hwc_blank(hwc_composer_device_1* dev, int disp, int blank) {
    return 0;
}

static int hwc_query(hwc_composer_device_1* dev, int what, int* value) {
    return 0;
}

static int hwc_device_close(hw_device_t* dev) {
    auto context = reinterpret_cast<HwcContext*>(dev);
    delete context;
    return 0;
}

static int hwc_get_display_configs(hwc_composer_device_1* dev, int disp,
                                   uint32_t* configs, size_t* numConfigs) {
  if (disp != 0) {
    return -EINVAL;
  }

  if (*numConfigs > 0) {
    // Config[0] will be passed in to getDisplayAttributes as the disp
    // parameter. The ARC display supports only 1 configuration.
    configs[0] = 0;
    *numConfigs = 1;
  }
  return 0;
}

static int hwc_get_display_attributes(hwc_composer_device_1* dev,
                                      int disp, uint32_t config,
                                      const uint32_t* attributes,
                                      int32_t* values) {
  if (disp != 0 || config != 0) {
    return -EINVAL;
  }

  DEFINE_AND_VALIDATE_HOST_CONNECTION();

  while (*attributes != HWC_DISPLAY_NO_ATTRIBUTE) {
    switch (*attributes) {
      case HWC_DISPLAY_VSYNC_PERIOD:
        *values = rcEnc->rcGetDisplayVsyncPeriod(rcEnc, disp);
        break;
      case HWC_DISPLAY_WIDTH:
        *values = rcEnc->rcGetDisplayWidth(rcEnc, disp);
        break;
      case HWC_DISPLAY_HEIGHT:
        *values = rcEnc->rcGetDisplayHeight(rcEnc, disp);
        break;
      case HWC_DISPLAY_DPI_X:
        *values = 1000 * rcEnc->rcGetDisplayDpiX(rcEnc, disp);
        break;
      case HWC_DISPLAY_DPI_Y:
        *values = 1000 * rcEnc->rcGetDisplayDpiY(rcEnc, disp);
        break;
      default:
        ALOGE("Unknown attribute value 0x%02x", *attributes);
    }
    ++attributes;
    ++values;
  }
  return 0;
}

static int hwc_device_open(const hw_module_t* module, const char* name, hw_device_t** device) {
    ALOGD("%s", __PRETTY_FUNCTION__);

    if (strcmp(name, HWC_HARDWARE_COMPOSER) != 0)
        return -EINVAL;

    auto dev = new HwcContext;
    dev->device.common.tag = HARDWARE_DEVICE_TAG;
    dev->device.common.version = HWC_DEVICE_API_VERSION_1_0;
    dev->device.common.module = const_cast<hw_module_t*>(module);
    dev->device.common.close = hwc_device_close;
    dev->device.prepare = hwc_prepare;
    dev->device.set = hwc_set;
    dev->device.eventControl = hwc_event_control;
    dev->device.blank = hwc_blank;
    dev->device.query = hwc_query;
    dev->device.getDisplayConfigs = hwc_get_display_configs;
    dev->device.getDisplayAttributes = hwc_get_display_attributes;
    dev->device.registerProcs = hwc_register_procs;
    dev->device.dump = nullptr;

    *device = &dev->device.common;

    return 0;
}

static hw_module_methods_t hwc_module_methods = {
    .open = hwc_device_open
};

hwc_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 1,
        .version_minor = 0,
        .id = HWC_HARDWARE_MODULE_ID,
        .name = "Hardware Composer Module",
        .author = "Anbox Developers",
        .methods = &hwc_module_methods,
    }
};
