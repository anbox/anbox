/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * Adapted for Anbox by Alec Barber 2021.
 */

#ifndef ANBOX_CAMERA_CAMERA_COMMON_H
#define ANBOX_CAMERA_CAMERA_COMMON_H

/*
 * Contains declarations of platform-independent the stuff that is used in
 * camera emulation.
 */

#include <cstdlib>
#include <cstdint>
#include <cerrno>
#include <linux/videodev2.h>
#include <memory>
#include <string>
#include <vector>

/*
 * These are missing in the current linux/videodev2.h
 */

#ifndef V4L2_PIX_FMT_YVYU
#define V4L2_PIX_FMT_YVYU    v4l2_fourcc('Y', 'V', 'Y', 'U')
#endif /* V4L2_PIX_FMT_YVYU */
#ifndef V4L2_PIX_FMT_VYUY
#define V4L2_PIX_FMT_VYUY    v4l2_fourcc('V', 'Y', 'U', 'Y')
#endif /* V4L2_PIX_FMT_VYUY */
#ifndef V4L2_PIX_FMT_YUY2
#define V4L2_PIX_FMT_YUY2    v4l2_fourcc('Y', 'U', 'Y', '2')
#endif /* V4L2_PIX_FMT_YUY2 */
#ifndef V4L2_PIX_FMT_YUNV
#define V4L2_PIX_FMT_YUNV    v4l2_fourcc('Y', 'U', 'N', 'V')
#endif /* V4L2_PIX_FMT_YUNV */
#ifndef V4L2_PIX_FMT_V422
#define V4L2_PIX_FMT_V422    v4l2_fourcc('V', '4', '2', '2')
#endif /* V4L2_PIX_FMT_V422 */
#ifndef V4L2_PIX_FMT_YYVU
#define V4L2_PIX_FMT_YYVU    v4l2_fourcc('Y', 'Y', 'V', 'U')
#endif /* V4L2_PIX_FMT_YYVU */
#ifndef V4L2_PIX_FMT_SGBRG8
#define V4L2_PIX_FMT_SGBRG8  v4l2_fourcc('G', 'B', 'R', 'G')
#endif  /* V4L2_PIX_FMT_SGBRG8 */
#ifndef V4L2_PIX_FMT_SGRBG8
#define V4L2_PIX_FMT_SGRBG8  v4l2_fourcc('G', 'R', 'B', 'G')
#endif  /* V4L2_PIX_FMT_SGRBG8 */
#ifndef V4L2_PIX_FMT_SRGGB8
#define V4L2_PIX_FMT_SRGGB8  v4l2_fourcc('R', 'G', 'G', 'B')
#endif  /* V4L2_PIX_FMT_SRGGB8 */
#ifndef V4L2_PIX_FMT_SBGGR10
#define V4L2_PIX_FMT_SBGGR10 v4l2_fourcc('B', 'G', '1', '\0')
#endif  /* V4L2_PIX_FMT_SBGGR10 */
#ifndef V4L2_PIX_FMT_SGBRG10
#define V4L2_PIX_FMT_SGBRG10 v4l2_fourcc('G', 'B', '1', '\0')
#endif  /* V4L2_PIX_FMT_SGBRG10 */
#ifndef V4L2_PIX_FMT_SGRBG10
#define V4L2_PIX_FMT_SGRBG10 v4l2_fourcc('B', 'A', '1', '\0')
#endif  /* V4L2_PIX_FMT_SGRBG10 */
#ifndef V4L2_PIX_FMT_SRGGB10
#define V4L2_PIX_FMT_SRGGB10 v4l2_fourcc('R', 'G', '1', '\0')
#endif  /* V4L2_PIX_FMT_SRGGB10 */
#ifndef V4L2_PIX_FMT_SBGGR12
#define V4L2_PIX_FMT_SBGGR12 v4l2_fourcc('B', 'G', '1', '2')
#endif  /* V4L2_PIX_FMT_SBGGR12 */
#ifndef V4L2_PIX_FMT_SGBRG12
#define V4L2_PIX_FMT_SGBRG12 v4l2_fourcc('G', 'B', '1', '2')
#endif  /* V4L2_PIX_FMT_SGBRG12 */
#ifndef V4L2_PIX_FMT_SGRBG12
#define V4L2_PIX_FMT_SGRBG12 v4l2_fourcc('B', 'A', '1', '2')
#endif  /* V4L2_PIX_FMT_SGRBG12 */
#ifndef V4L2_PIX_FMT_SRGGB12
#define V4L2_PIX_FMT_SRGGB12 v4l2_fourcc('R', 'G', '1', '2')
#endif  /* V4L2_PIX_FMT_SRGGB12 */

#define UNUSED(x) (void)(x)

namespace anbox::camera {

/* Describes framebuffer, used by the client of camera capturing API.
 * This descritptor is used in camera_device_read_frame call.
 */
typedef struct ClientFrameBuffer {
    /* Pixel format used in the client framebuffer. */
    uint32_t    pixel_format;
    /* Address of the client framebuffer. */
    void*       framebuffer;
} ClientFrameBuffer;

/* Describes frame dimensions.
 */
typedef struct CameraFrameDim {
    /* Frame width. */
    int     width;
    /* Frame height. */
    int     height;
} CameraFrameDim;

/* Camera information descriptor, containing properties of a camera connected
 * to the host.
 *
 * Instances of this structure are created during camera device enumerations,
 * and are considered to be constant everywhere else. The only exception to this
 * rule is changing the 'in_use' flag during creation / destruction of a service
 * representing that camera.
 */
typedef struct CameraInfo {
    /* User-friendly camera display name. */
    std::string                 display_name;
    /* Device name for the camera. */
    std::string                 device_name;
    /* Input channel for the camera. */
    int                         inp_channel;
    /* Pixel format chosen for the camera. */
    uint32_t                    pixel_format;
    /* Direction the camera is facing: 'front', or 'back' */
    std::string                 direction;
    /* Array of frame sizes supported for the pixel format chosen for the camera. */
    std::shared_ptr<std::vector<CameraFrameDim>> frame_sizes;
    /* In use status. When there is a camera service created for this camera,
     * "in use" is set to true. Otherwise this flag is zet to false. */
    bool                        in_use;
} CameraInfo;

/* Returns current time in microseconds. */
static __inline__ uint64_t
_get_timestamp(void)
{
    struct timeval t;
    t.tv_sec = t.tv_usec = 0;
    gettimeofday(&t, NULL);
    return static_cast<uint64_t>(t.tv_sec) * 1000000LL + t.tv_usec;
}

/* Sleeps for the given amount of milliseconds */
static __inline__ void
_camera_sleep(uint64_t millisec)
{
    struct timeval t;
    const uint64_t wake_at = _get_timestamp() + millisec * 1000;
    do {
        const uint64_t stamp = _get_timestamp();
        if ((stamp / 1000) >= (wake_at / 1000)) {
            break;
        }
        t.tv_sec = (wake_at - stamp) / 1000000;
        t.tv_usec = (wake_at - stamp) - static_cast<uint64_t>(t.tv_sec) * 1000000;
    } while (select(0, NULL, NULL, NULL, &t) < 0 && errno == EINTR);
}

}

#endif  /* ANBOX_CAMERA_CAMERA_COMMON_H */
