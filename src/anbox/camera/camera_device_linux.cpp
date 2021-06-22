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

/*
 * Contains code that is used to capture video frames from a camera device
 * on Linux. This code uses V4L2 API to work with camera devices, and requires
 * Linux kernel version at least 2.5
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "anbox/do_not_copy_or_move.h"
#include "anbox/logger.h"
#include "anbox/camera/camera_device.h"
#include "anbox/camera/camera_format_converters.h"

#define  E(...)    ERROR(__VA_ARGS__)
#define  W(...)    WARNING(__VA_ARGS__)
#define  D(...)    DEBUG(__VA_ARGS__)
#define  D_ACTIVE  1

#define CLEAR(x) memset (&(x), 0, sizeof(x))

namespace {

/* Pixel format descriptor.
 * Instances of this descriptor are created during camera device enumeration,
 * and an instance of this structure describing pixel format chosen for the
 * camera emulation is saved by the camera factory service to represent an
 * emulating camera properties.
 */
struct QemuPixelFormat {
  /* Pixel format in V4L2_PIX_FMT_XXX form. */
  uint32_t                                   format;
  /* Frame dimensions supported by this format. */
  std::shared_ptr<std::vector<anbox::camera::CameraFrameDim>> dims;
};

/* Describes a framebuffer. */
typedef struct CameraFrameBuffer {
  /* Framebuffer data. */
  uint8_t*    data;
  /* Framebuffer data size. */
  size_t      size;
} CameraFrameBuffer;

/* Defines type of the I/O used to obtain frames from the device. */
enum CameraIoType {
  /* Framebuffers are shared via memory mapping. */
  CAMERA_IO_MEMMAP,
  /* Framebuffers are available via user pointers. */
  CAMERA_IO_USERPTR,
  /* Framebuffers are to be read from the device. */
  CAMERA_IO_DIRECT
};

/* Preferred pixel formats arranged from the most to the least desired.
 *
 * More than anything else this array is defined by an existance of format
 * conversion between the camera supported formats, and formats that are
 * supported by camera framework in the guest system. Currently, guest supports
 * only YV12 pixel format for data, and RGB32 for preview. So, this array should
 * contain only those formats, for which converters are implemented. Generally
 * speaking, the order in which entries should be arranged in this array matters
 * only as far as conversion speed is concerned. So, formats with the fastest
 * converters should be put closer to the top of the array, while slower ones
 * should be put closer to the bottom. But as far as functionality is concerned,
 * the orser doesn't matter, and any format can be placed anywhere in this array,
 * as long as conversion for it exists.
 */
static const std::vector<uint32_t> _preferred_formats =
{
  /* Native format for the emulated camera: no conversion at all. */
  V4L2_PIX_FMT_YUV420,
  V4L2_PIX_FMT_YVU420,
  /* Continue with YCbCr: less math than with RGB */
  V4L2_PIX_FMT_NV12,
  V4L2_PIX_FMT_NV21,
  V4L2_PIX_FMT_YUYV,
  /* End with RGB. */
  V4L2_PIX_FMT_RGB32,
  V4L2_PIX_FMT_RGB24,
  V4L2_PIX_FMT_RGB565,
};

/*******************************************************************************
 *                     Helper routines
 ******************************************************************************/

/* IOCTL wrapper. */
static int _xioctl(int fd, int request, void *arg) {
 int r;
 do {
     r = ioctl(fd, request, arg);
 } while (-1 == r && EINTR == errno);
 return r;
}

/* Returns an index of the given pixel format in an array containing pixel
 * format descriptors.
 * This routine is used to choose a pixel format for a camera device. The idea
 * is that when the camera service enumerates all pixel formats for all cameras
 * connected to the host, we need to choose just one, which would be most
 * appropriate for camera emulation. To do that, the camera service will run
 * formats, contained in _preferred_formats array against enumerated pixel
 * formats to pick the first format that match.
 * Param:
 *  fmt - Pixel format, for which to obtain the index.
 *  formats - Array containing list of pixel formats, supported by the camera
 *      device.
 * Return:
 *  Index of the matched entry in the array, or -1 if no entry has been found.
 */
static int _get_format_index(uint32_t fmt,
                             const std::vector<QemuPixelFormat> &formats) {
  size_t f;
  for (f = 0; f < formats.size() && formats[f].format != fmt; f++);
  return f < formats.size() ? f : -1;
}

/*******************************************************************************
 *                     CameraFrameBuffer routines
 ******************************************************************************/

/* Frees array of framebuffers, depending on the I/O method the array has been
 * initialized for.
 * Note that this routine doesn't frees the array itself.
 * Param:
 *  fbs - Array data
 *  io_type - Type of the I/O the array has been initialized for.
 */
static void _free_framebuffers(std::vector<CameraFrameBuffer> fbs,
                               CameraIoType io_type) {
  switch (io_type) {
   case CAMERA_IO_MEMMAP:
    /* Unmap framebuffers. */
    for (auto fb : fbs) {
        if (fb.data != NULL) {
            munmap(fb.data, fb.size);
            fb.data = NULL;
            fb.size = 0;
        }
    }
    break;

   case CAMERA_IO_USERPTR:
   case CAMERA_IO_DIRECT:
    /* Free framebuffers. */
    for (auto fb : fbs) {
        if (fb.data != NULL) {
            free(fb.data);
            fb.data = NULL;
            fb.size = 0;
        }
    }
    break;

   default:
    E("%s: Invalid I/O type %d", __FUNCTION__, io_type);
    break;
  }
}

}

namespace anbox::camera {

/*******************************************************************************
 *                     CameraDevice routines
 ******************************************************************************/

/*
 * Describes a connection to an actual camera device.
 */
class LinuxCameraDevice : public CameraDevice {
 public:
  /* Camera device name. (default is /dev/video0) */
  char*                       device_name;
  /* Input channel. (default is 0) */
  int                         input_channel;

  /*
   * Set by the framework after initializing camera connection.
   */

  /* Handle to the opened camera device. */
  int                         handle;
  /* Device capabilities. */
  struct v4l2_capability      caps;
  /* Actual pixel format reported by the device when capturing is started. */
  struct v4l2_pix_format      actual_pixel_format;
  /* Defines type of the I/O to use to retrieve frames from the device. */
  CameraIoType                io_type;
  /* Allocated framebuffers. */
  std::vector<CameraFrameBuffer> framebuffers;

 public:

  /* Initialises an instance of LinuxCameraDevice structure. Note that this
   *  routine also sets 'opaque' field in the 'header' structure to point back
   *  to the containing LinuxCameraDevice instance.
   */
  LinuxCameraDevice();

  /* Uninitializes and frees CameraDevice structure.
   */
  ~LinuxCameraDevice();

  /* Resets camera device after capturing.
   * Since new capture request may require different frame dimensions we must
   * reset camera device by reopening its handle. Otherwise attempts to set up
   * new frame properties (different from the previous one) may fail.
   */
  void reset();

  /* Memory maps buffers and shares mapped memory with the device.
   * Return:
   *  0 Framebuffers have been mapped.
   *  -1 A critical error has ocurred.
   *  1 Memory mapping is not available.
   */
  int mmapFramebuffer();

  /* Allocates frame buffers and registers them with the device.
   * Return:
   *  0 Framebuffers have been mapped.
   *  -1 A critical error has ocurred.
   *  1 Device doesn't support user pointers.
   */
  int userFramebuffer();

  /* Allocate frame buffer for direct read from the device.
   * Return:
   *  0 Framebuffers have been mapped.
   *  -1 A critical error has ocurred.
   *  1 Memory mapping is not available.
   */
  int directFramebuffer();

  /* Opens camera device.
   * Return:
   *  0 on success, != 0 on failure.
   */
  int openDevice();

  /* Enumerates frame sizes for the given pixel format.
   * Param:
   *  fmt - Pixel format to enum frame sizes for.
   * Return:
   *  On success returns an array of frame sizes. On failure returns NULL.
   */
  std::shared_ptr<std::vector<CameraFrameDim>> enumFormatSizes(uint32_t fmt);


  /* Enumerates pixel formats, supported by the device.
   * Note that this routine will enumerate only raw (uncompressed) formats.
   * Return:
   *  On success returns an array of pixel formats. On failure returns NULL.
   */
  std::shared_ptr<std::vector<QemuPixelFormat>> enumPixelFormats();

  /* Collects information about an opened camera device.
   * The information collected in this routine contains list of pixel formats,
   * supported by the device, and list of frame dimensions supported by the
   * camera for each pixel format.
   * Param:
   *  cis - Upon success contains information collected from the camera device.
   * Return:
   *  0 on success, != 0 on failure.
   */
  int getInfo(CameraInfo* cis);

 public:
  // Inherited interface
  virtual int startCapturing(uint32_t pixel_format,
                             unsigned int frame_width,
                             unsigned int frame_height);
      
  virtual int stopCapturing();

  virtual int readFrame(ClientFrameBuffer* framebuffers,
                        int fbs_num,
                        float r_scale,
                        float g_scale,
                        float b_scale,
                        float exp_comp);
      
  virtual void closeCameraDevice();

};

CameraDevice::~CameraDevice() {}

LinuxCameraDevice::LinuxCameraDevice() : 
    device_name(NULL), input_channel(0), handle(-1), caps(),
    actual_pixel_format(), io_type(), framebuffers() {}

LinuxCameraDevice::~LinuxCameraDevice() {
  /* Closing handle will also disconnect from the driver. */
  if (handle >= 0) {
    close(handle);
  }
  if (device_name != NULL) {
    free(device_name);
  }
  _free_framebuffers(framebuffers, io_type);
}

void LinuxCameraDevice::reset() {
  struct v4l2_cropcap cropcap;
  struct v4l2_crop crop;

  /* Free capturing framebuffers first. */
  if (!framebuffers.empty()) {
    _free_framebuffers(framebuffers, io_type);
    framebuffers.clear();
  }

  /* Reset device handle. */
  close(handle);
  handle = open(device_name, O_RDWR | O_NONBLOCK, 0);

  if (handle >= 0) {
    /* Select video input, video standard and tune here. */
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    _xioctl(handle, static_cast<int>(VIDIOC_CROPCAP), &cropcap);
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect; /* reset to default */
    _xioctl (handle, static_cast<int>(VIDIOC_S_CROP), &crop);
  }
}

int LinuxCameraDevice::mmapFramebuffer()
{
  struct v4l2_requestbuffers req;
  CLEAR(req);
  req.count   = 4;
  req.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory  = V4L2_MEMORY_MMAP;

  /* Request memory mapped buffers. Note that device can return less buffers
   * than requested. */
  if(_xioctl(handle, static_cast<int>(VIDIOC_REQBUFS), &req)) {
    if (EINVAL == errno) {
      D("Device '%s' does not support memory mapping", device_name);
      return 1;
    } else {
      E("static_cast<int>(VIDIOC_REQBUFS) has failed: %s", strerror(errno));
      return -1;
    }
  }

  /* Allocate framebuffer array. */
  framebuffers.resize(req.count);

  /* Map every framebuffer to the shared memory, and queue it
   * with the device. */
  for(size_t i = 0; i < framebuffers.size(); i++) {
    /* Map framebuffer. */
    struct v4l2_buffer buf;
    CLEAR(buf);
    buf.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory  = V4L2_MEMORY_MMAP;
    buf.index   = i;
    if(_xioctl(handle, static_cast<int>(VIDIOC_QUERYBUF), &buf) < 0) {
      E("static_cast<int>(VIDIOC_QUERYBUF) has failed: %s", strerror(errno));
      return -1;
    }
    framebuffers[i].size = buf.length;
    framebuffers[i].data = reinterpret_cast<uint8_t*>(
      mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED,
           handle, buf.m.offset));
    if (MAP_FAILED == framebuffers[i].data) {
      E("Memory mapping has failed: %s", strerror(errno));
      return -1;
    }

    /* Queue the mapped buffer. */
    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
    if (_xioctl(handle, static_cast<int>(VIDIOC_QBUF), &buf) < 0) {
      E("static_cast<int>(VIDIOC_QBUF) has failed: %s", strerror(errno));
      return -1;
    }
  }

  io_type = CAMERA_IO_MEMMAP;

  return 0;
}

int LinuxCameraDevice::userFramebuffer() {
  struct v4l2_requestbuffers req;
  CLEAR (req);
  req.count   = 4;
  req.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory  = V4L2_MEMORY_USERPTR;

  /* Request user buffers. Note that device can return less buffers
   * than requested. */
  if(_xioctl(handle, static_cast<int>(VIDIOC_REQBUFS), &req)) {
    if (EINVAL == errno) {
      D("Device '%s' does not support user pointers", device_name);
      return 1;
    } else {
      E("static_cast<int>(VIDIOC_REQBUFS) has failed: %s", strerror(errno));
      return -1;
    }
  }

  /* Allocate framebuffer array. */
  framebuffers.resize(req.count);

  /* Allocate buffers, queueing them wit the device at the same time */
  for(size_t i = 0; i < framebuffers.size(); i++) {
    framebuffers[i].size = actual_pixel_format.sizeimage;
    framebuffers[i].data = static_cast<uint8_t*>(malloc(framebuffers[i].size));
    if (framebuffers[i].data == NULL) {
      E("Not enough memory to allocate framebuffer");
      return -1;
    }

    /* Queue the user buffer. */
    struct v4l2_buffer buf;
    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_USERPTR;
    buf.m.userptr = reinterpret_cast<unsigned long>(framebuffers[i].data);
    buf.length = framebuffers[i].size;
    if (_xioctl(handle, static_cast<int>(VIDIOC_QBUF), &buf) < 0) {
      E("static_cast<int>(VIDIOC_QBUF) has failed: %s", strerror(errno));
      return -1;
    }
  }

  io_type = CAMERA_IO_USERPTR;

  return 0;
}

int LinuxCameraDevice::directFramebuffer() {
  /* Allocate framebuffer array. */
  framebuffers.resize(1);

  framebuffers[0].size = actual_pixel_format.sizeimage;
  framebuffers[0].data = static_cast<uint8_t*>(malloc(framebuffers[0].size));
  if (framebuffers[0].data == NULL) {
    E("Not enough memory to allocate framebuffer");
    return -1;
  }

  io_type = CAMERA_IO_DIRECT;

  return 0;
}

int LinuxCameraDevice::openDevice() {
  struct stat st;

  if (stat(device_name, &st)) {
    return -1;
  }

  if (!S_ISCHR(st.st_mode)) {
    E("'%s' is not a device", device_name);
    return -1;
  }

  /* Open handle to the device, and query device capabilities. */
  handle = open(device_name, O_RDWR | O_NONBLOCK, 0);
  if (handle < 0) {
    E("Cannot open camera device '%s': %s", device_name, strerror(errno));
    return -1;
  }
  if (_xioctl(handle, static_cast<int>(VIDIOC_QUERYCAP), &caps) < 0) {
    if (EINVAL == errno) {
      E("Camera '%s' is not a V4L2 device", device_name);
      close(handle);
      handle = -1;
      return -1;
    } else {
      E("Unable to query capabilities for camera device '%s'", device_name);
      close(handle);
      handle = -1;
      return -1;
    }
  }

  /* Make sure that camera supports minimal requirements. */
  if (!(caps.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    E("Camera '%s' is not a video capture device", device_name);
    close(handle);
    handle = -1;
    return -1;
  }

  return 0;
}

std::shared_ptr<std::vector<CameraFrameDim>>
LinuxCameraDevice::enumFormatSizes(uint32_t fmt) {
  int n;
  int sizes_num = 0;
  int out_num = 0;
  struct v4l2_frmsizeenum size_enum;
  const auto result = std::make_shared<std::vector<CameraFrameDim>>();

  /* Calculate number of supported sizes for the given format. */
  for (n = 0; ; n++) {
    size_enum.index = n;
    size_enum.pixel_format = fmt;
    if(_xioctl(handle, static_cast<int>(VIDIOC_ENUM_FRAMESIZES), &size_enum)) {
      break;
    }
    if (size_enum.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
      /* Size is in the simpe width, height form. */
      sizes_num++;
    } else if (size_enum.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
      /* Sizes are represented as min/max width and height with a step for
       * each dimension. Since at the end we want to list each supported
       * size in the array (that's the only format supported by the guest
       * camera framework), we need to calculate how many array entries
       * this will generate. */
      const uint32_t dif_widths =
          (size_enum.stepwise.max_width - size_enum.stepwise.min_width) /
          size_enum.stepwise.step_width + 1;
      const uint32_t dif_heights =
          (size_enum.stepwise.max_height - size_enum.stepwise.min_height) /
          size_enum.stepwise.step_height + 1;
      sizes_num += dif_widths * dif_heights;
    } else if (size_enum.type == V4L2_FRMSIZE_TYPE_CONTINUOUS) {
      /* Special stepwise case, when steps are set to 1. We still need to
       * flatten this for the guest, but the array may be too big.
       * Fortunately, we don't need to be fancy, so three sizes would be
       * sufficient here: min, max, and one in the middle. */
      sizes_num += 3;
    }

  }
  if (sizes_num == 0) {
      return result;
  }

  /* Allocate, and initialize the array of supported entries. */
  result->resize(sizes_num);
  for (n = 0; out_num < sizes_num; n++) {
    size_enum.index = n;
    size_enum.pixel_format = fmt;
    if(_xioctl(handle, static_cast<int>(VIDIOC_ENUM_FRAMESIZES), &size_enum)) {
      /* Errors are not welcome here anymore. */
      E("Unexpected failure while getting pixel dimensions: %s",
        strerror(errno));
      return nullptr;
    }

    if (size_enum.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
      result->at(out_num).width = size_enum.discrete.width;
      result->at(out_num).height = size_enum.discrete.height;
      out_num++;
    } else if (size_enum.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
      uint32_t w;
      for (w = size_enum.stepwise.min_width;
           w <= size_enum.stepwise.max_width;
           w += size_enum.stepwise.step_width) {
        uint32_t h;
        for (h = size_enum.stepwise.min_height;
           h <= size_enum.stepwise.max_height;
           h += size_enum.stepwise.step_height) {
          result->at(out_num).width = w;
          result->at(out_num).height = h;
          out_num++;
        }
      }
    } else if (size_enum.type == V4L2_FRMSIZE_TYPE_CONTINUOUS) {
      /* min */
      result->at(out_num).width = size_enum.stepwise.min_width;
      result->at(out_num).height = size_enum.stepwise.min_height;
      out_num++;
      /* one in the middle */
      result->at(out_num).width =
          (size_enum.stepwise.min_width + size_enum.stepwise.max_width) / 2;
      result->at(out_num).height =
          (size_enum.stepwise.min_height + size_enum.stepwise.max_height) / 2;
      out_num++;
      /* max */
      result->at(out_num).width = size_enum.stepwise.max_width;
      result->at(out_num).height = size_enum.stepwise.max_height;
      out_num++;
    }
  }

  return result;
}

std::shared_ptr<std::vector<QemuPixelFormat>>
LinuxCameraDevice::enumPixelFormats() {
  int n, max_fmt;
  int fmt_num = 0;
  int out_num = 0;
  struct v4l2_fmtdesc fmt_enum;
  const auto result = std::make_shared<std::vector<QemuPixelFormat>>();

  /* Calculate number of supported formats. */
  for (max_fmt = 0; ; max_fmt++) {
    memset(&fmt_enum, 0, sizeof(fmt_enum));
    fmt_enum.index = max_fmt;
    fmt_enum.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(_xioctl(handle, static_cast<int>(VIDIOC_ENUM_FMT), &fmt_enum)) {
        break;
    }
    /* Skip the compressed ones. */
    if ((fmt_enum.flags & V4L2_FMT_FLAG_COMPRESSED) == 0) {
        fmt_num++;
    }
  }
  if (fmt_num == 0) {
    return 0;
  }

  /* Allocate, and initialize array for enumerated formats. */
  result->resize(fmt_num);
  for (n = 0; n < max_fmt && out_num < fmt_num; n++) {
    memset(&fmt_enum, 0, sizeof(fmt_enum));
    fmt_enum.index = n;
    fmt_enum.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(_xioctl(handle, static_cast<int>(VIDIOC_ENUM_FMT), &fmt_enum)) {
      /* Errors are not welcome here anymore. */
      E("Unexpected failure while getting pixel format: %s", strerror(errno));
        return nullptr;
    }
    /* Skip the compressed ones. */
    if ((fmt_enum.flags & V4L2_FMT_FLAG_COMPRESSED) == 0) {
      result->at(out_num).format = fmt_enum.pixelformat;
      /* Enumerate frame dimensions supported for this format. */
      result->at(out_num).dims = enumFormatSizes(fmt_enum.pixelformat);
      if (result->at(out_num).dims) {
        out_num++;
      } else {
        E("Unable to enumerate supported dimensions for pixel format %d",
          fmt_enum.pixelformat);
        return nullptr;
      }
    }
  }

  return result;
}

int LinuxCameraDevice::getInfo(CameraInfo* cis) {
  int chosen = -1;
  const auto formats = enumPixelFormats();
  if (!formats) {
    return -1;
  }

  /* Lets see if camera supports preferred formats */
  for (auto f : _preferred_formats) {
    chosen = _get_format_index(f, *formats);
    if (chosen >= 0) {
      break;
    }
  }
  if (chosen < 0) {
    /* Camera doesn't support any of the chosen formats. Then it doesn't
     * matter which one we choose. Lets choose the first one. */
    chosen = 0;
  }

  cis->device_name = strdup(device_name);
  cis->inp_channel = input_channel;
  cis->pixel_format = formats->at(chosen).format;
  cis->frame_sizes = formats->at(chosen).dims;
  cis->in_use = false;

  return 0;
}

int LinuxCameraDevice::startCapturing(uint32_t pixel_format,
                                      unsigned int frame_width,
                                      unsigned int frame_height) {
  struct v4l2_format fmt;
  char fmt_str[5];
  int r;

  /* Sanity checks. */
  if (handle < 0) {
    E("Camera device is not opened");
    return -1;
  }

  /* Try to set pixel format with the given dimensions. */
  CLEAR(fmt);
  fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width       = frame_width;
  fmt.fmt.pix.height      = frame_height;
  fmt.fmt.pix.pixelformat = pixel_format;
  if (_xioctl(handle, static_cast<int>(VIDIOC_S_FMT), &fmt) < 0) {
    memcpy(fmt_str, &pixel_format, 4);
    fmt_str[4] = '\0';
    E("Camera '%s' does not support pixel format '%s' with dimensions %dx%d",
      device_name, fmt_str, frame_width, frame_height);
    reset();
    return -1;
  }
  /* static_cast<int>(VIDIOC_S_FMT) may has changed some properties of the
   * structure. Make sure that dimensions didn't change.
   */
  if (fmt.fmt.pix.width != frame_width || fmt.fmt.pix.height != frame_height) {
    memcpy(fmt_str, &pixel_format, 4);
    fmt_str[4] = '\0';
    E("Dimensions %dx%d are wrong for pixel format '%s'",
      frame_width, frame_height, fmt_str);
    reset();
    return -1;
  }
  memcpy(&actual_pixel_format, &fmt.fmt.pix, sizeof(struct v4l2_pix_format));

  /* Let's initialize frame buffers, and see what kind of I/O we're going to
   * use to retrieve frames.
   */

  /* First, lets see if we can do mapped I/O (as most performant one). */
  r = mmapFramebuffer();
  if (r < 0) {
    /* Some critical error has ocurred. Bail out. */
    reset();
    return -1;
  } else if (r > 0) {
    /* Device doesn't support memory mapping. Retrieve to the next performant
     * one: preallocated user buffers. */
    r = userFramebuffer();
    if (r < 0) {
      /* Some critical error has ocurred. Bail out. */
      reset();
      return -1;
    } else if (r > 0) {
      /* The only thing left for us is direct reading from the device. */
      if (!(caps.capabilities & V4L2_CAP_READWRITE)) {
        E("Don't know how to access frames on device '%s'", device_name);
        reset();
        return -1;
      }
      r = directFramebuffer();
      if (r != 0) {
        /* Any error at this point is a critical one. */
        reset();
        return -1;
      }
    }
  }

  /* Start capturing from the device. */
  if (io_type != CAMERA_IO_DIRECT) {
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (_xioctl (handle, static_cast<int>(VIDIOC_STREAMON), &type) < 0) {
      E("static_cast<int>(VIDIOC_STREAMON) on camera '%s' has failed: %s",
        device_name, strerror(errno));
      reset();
      return -1;
    }
  }
  return 0;
}

int LinuxCameraDevice::stopCapturing() {
  enum v4l2_buf_type type;

  /* Sanity checks. */
  if (handle < 0) {
    E("Camera device is not opened");
    return -1;
  }

  switch (io_type) {
   case CAMERA_IO_DIRECT:
    /* Nothing to do. */
    break;

   case CAMERA_IO_MEMMAP:
   case CAMERA_IO_USERPTR:
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (_xioctl(handle, static_cast<int>(VIDIOC_STREAMOFF), &type) < 0) {
      E("static_cast<int>(VIDIOC_STREAMOFF) on camera '%s' has failed: %s",
        device_name, strerror(errno));
      return -1;
    }
    break;
   default:
    E("Unknown I/O method: %d", io_type);
    return -1;
  }

  /* Reopen the device to reset its internal state. It seems that if we don't
   * do that, an attempt to reinit the device with different frame dimensions
   * would fail. */
  reset();

  return 0;
}

int LinuxCameraDevice::readFrame(ClientFrameBuffer* framebuffers,
                                 int fbs_num,
                                 float r_scale,
                                 float g_scale,
                                 float b_scale,
                                 float exp_comp) {
  /* Sanity checks. */
  if (handle < 0) {
    E("Camera device is not opened");
    return -1;
  }

  if (io_type == CAMERA_IO_DIRECT) {
    /* Read directly from the device. */
    size_t total_read_bytes = 0;
    /* There is one framebuffer allocated for direct read. */
    uint8_t* buff = this->framebuffers[0].data;
    do {
      int read_bytes =
        read(handle, buff + total_read_bytes,
             actual_pixel_format.sizeimage - total_read_bytes);
      if (read_bytes < 0) {
        switch (errno) {
         case EIO:
         case EAGAIN:
          continue;
         default:
          E("Unable to read from the camera device '%s': %s",
            device_name, strerror(errno));
          return -1;
        }
      }
      total_read_bytes += read_bytes;
    } while (total_read_bytes < actual_pixel_format.sizeimage);
    /* Convert the read frame into the caller's framebuffers. */
    return convert_frame(buff, actual_pixel_format.pixelformat,
                         actual_pixel_format.sizeimage,
                         actual_pixel_format.width,
                         actual_pixel_format.height,
                         framebuffers, fbs_num,
                         r_scale, g_scale, b_scale, exp_comp);
  } else {
    /* Dequeue next buffer from the device. */
    struct v4l2_buffer buf;
    int res;
    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = io_type == CAMERA_IO_MEMMAP ? V4L2_MEMORY_MMAP :
                                                   V4L2_MEMORY_USERPTR;
    for (;;) {
      const int res = _xioctl(handle, static_cast<int>(VIDIOC_DQBUF), &buf);
      if (res >= 0) {
        break;
      } else if (errno == EAGAIN) {
        return 1;   // Tells the caller to repeat.
      } else if (errno != EINTR && errno != EIO) {
        E("static_cast<int>(VIDIOC_DQBUF) on camera '%s' has failed: %s",
          device_name, strerror(errno));
        return -1;
      }
    }

    /* Convert frame to the receiving buffers. */
    res = convert_frame(this->framebuffers[buf.index].data,
                        actual_pixel_format.pixelformat,
                        actual_pixel_format.sizeimage,
                        actual_pixel_format.width,
                        actual_pixel_format.height,
                        framebuffers, fbs_num,
                        r_scale, g_scale, b_scale, exp_comp);

    /* Requeue the buffer back to the device. */
    if (_xioctl(handle, static_cast<int>(VIDIOC_QBUF), &buf) < 0) {
      W("static_cast<int>(VIDIOC_QBUF) on camera '%s' has failed: %s",
        device_name, strerror(errno));
    }

    return res;
  }
}

void LinuxCameraDevice::closeCameraDevice() {
  // No-op
}

/*******************************************************************************
 *                     CameraDevice API
 ******************************************************************************/

std::shared_ptr<CameraDevice> CameraDevice::openCameraDevice(const char* name,
                                                             int inp_channel) {
  struct v4l2_cropcap cropcap;
  struct v4l2_crop crop;
  std::shared_ptr<LinuxCameraDevice> cd = std::make_shared<LinuxCameraDevice>();

  /* Allocate and initialize the descriptor. */
  cd->device_name = name != NULL ? strdup(name) : strdup("/dev/video0");
  cd->input_channel = inp_channel;

  /* Open the device. */
  if (cd->openDevice()) {
    return NULL;
  }

  /* Select video input, video standard and tune here. */
  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  _xioctl(cd->handle, static_cast<int>(VIDIOC_CROPCAP), &cropcap);
  crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  crop.c = cropcap.defrect; /* reset to default */
  _xioctl (cd->handle, static_cast<int>(VIDIOC_S_CROP), &crop);

  return cd;
}

std::shared_ptr<std::vector<CameraInfo>>
CameraDevice::enumerateCameraDevices(int max) {
  char dev_name[24];
  const auto result = std::make_shared<std::vector<CameraInfo>>();

  for (int n = 0; n < max; n++) {
    sprintf(dev_name, "/dev/video%d", n);
    const auto cd = openCameraDevice(dev_name, 0);
    if (cd) {
      LinuxCameraDevice* lcd = dynamic_cast<LinuxCameraDevice*>(cd.get());
      CameraInfo ci;
      if (!lcd->getInfo(&ci)) {
        char user_name[24];
        sprintf(user_name, "webcam%d", n);
        ci.display_name = strdup(user_name);
        ci.in_use = 0;
        result->push_back(ci);
      }
      cd->closeCameraDevice();
    } else {
        break;
    }
  }

  return result;
}

}
