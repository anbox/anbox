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

#include "anbox/camera/camera_device.h"
#include "anbox/camera/camera_common.h"
#include "anbox/camera/camera_format_converters.h"
#include "anbox/camera/camera_service.h"
#include "anbox/qemu/camera_message_processor.h"
#include "anbox/logger.h"
#include <sstream>
#include <cassert>
#include <cstdlib>
#include <iostream>

#include <ctime>

using anbox::camera::CameraService;
using anbox::camera::CameraDevice;
using anbox::camera::has_converter;

namespace {
class CameraQuery {
public:
  CameraQuery(std::string query);

  std::string query_name;
  std::string query_param;

  std::map<std::string, std::string> query_param_map();
};

CameraQuery::CameraQuery(std::string query) {
  const auto qend = query.find(' ');

  query_name = query.substr(0, qend);

  if (qend != std::string::npos) {
    query_param = query.substr(qend + 1);
  } else {
    query_param = std::string();
  }
}

std::string _camera_info_to_string(const anbox::camera::CameraInfo &ci) {
  std::stringstream ss;

  /* Append device name. */
  ss << "name=" << ci.device_name << " ";

  /* Append input channel. */
  ss << "channel=" <<  ci.inp_channel << " ";
  
  /* Append pixel format. */
  ss << "pix=" << ci.pixel_format << " ";
  
  /* Append direction. */
  ss << "dir=" << ci.direction << " ";
  
  /* Append supported frame sizes. */
  ss << "framedims=" << ci.frame_sizes->at(0).width << "x" << ci.frame_sizes->at(0).height;
  
  for (size_t n = 1; n < ci.frame_sizes->size(); n++) {
    ss << "," << ci.frame_sizes->at(n).width << "x" << ci.frame_sizes->at(n).height;
  }

  /* Stringified camera properties should end with EOL. */
  ss << "\n";
  return ss.str();
}

std::shared_ptr<std::string> _get_token_value(const std::string &param, const std::string &name) {
  const auto search_term = name + "=";
  auto idx = std::string::npos;
  do {
    idx = param.find(search_term, idx + 1);
    if (idx == 0 || (idx != std::string::npos && param[idx - 1] == ' ')) {
      const auto start_idx = idx + search_term.size();
      const auto end_idx = param.find(' ', start_idx);
      if (start_idx == param.size()) {
        return std::make_shared<std::string>();
      } else if (end_idx == std::string::npos) {
        return std::make_shared<std::string>(param, start_idx);
      } else {
        return std::make_shared<std::string>(param, start_idx, end_idx - start_idx);
      }
    }
  } while (idx != std::string::npos);
  
  return NULL;
}

std::shared_ptr<long> _get_token_value_int(const std::string &param, const std::string &name) {
  const auto token = _get_token_value(param, name);
  if (token) {
    errno = 0;
    long value = strtol(token->c_str(), NULL, 10);
    if (errno) {
      ERROR("Value '%s' of the parameter '%s' in '%s' is not a decimal number.",
            token->c_str(), name.c_str(), param.c_str());
      return NULL;
    }
    return std::make_shared<long>(value);
  } else {
    return NULL;
  }
}

}

/*
 * Prefixes for replies to camera client queries.
 */

/* Success, no data to send in reply. */
#define OK_REPLY        "ok"
/* Failure, no data to send in reply. */
#define KO_REPLY        "ko"
/* Success, there are data to send in reply. */
#define OK_REPLY_DATA   OK_REPLY ":"
/* Failure, there are data to send in reply. */
#define KO_REPLY_DATA   KO_REPLY ":"

namespace anbox::qemu {
class CameraMessageProcessor::ConnectionParameters : public anbox::DoNotCopyOrMove {
 public:
  /* Client name.
   *  On Linux this is the name of the camera device.
   *  On Windows this is the name of capturing window.
   */
  std::string device_name;
  /* Input channel to use to connect to the camera. */
  int                 inp_channel;
  /* Camera information. */
  const anbox::camera::CameraInfo*   camera_info;
  /* Emulated camera device descriptor. */
  std::shared_ptr<CameraDevice>       camera;
  /* Buffer allocated for video frames.
   * Note that memory allocated for this buffer
   * also contains preview framebuffer. */
  uint8_t*            video_frame;
  /* Preview frame buffer.
   * This address points inside the 'video_frame' buffer. */
  uint16_t*           preview_frame;
  /* Byte size of the videoframe buffer. */
  size_t              video_frame_size;
  /* Byte size of the preview frame buffer. */
  size_t              preview_frame_size;
  /* Pixel format required by the guest. */
  uint32_t            pixel_format;
  /* Frame width. */
  int                 width;
  /* Frame height. */
  int                 height;
  /* Number of pixels in a frame buffer. */
  int                 pixel_num;
  /* Status of video and preview frame cache. */
  int                 frames_cached;
};

CameraMessageProcessor::CameraMessageProcessor(
    const std::shared_ptr<network::SocketMessenger> &messenger,
    const std::string &params)
    : messenger_(messenger) {
  /* Parse parameter string, containing camera client properties.
   */

  /* Pull optional device name. */
  const auto device_name_ptr = _get_token_value(params, "name");
  if (!device_name_ptr) {
    DEBUG("'name' parameter is missing, or misformed in '%s'. Running message processor in factory mode.",
      params.c_str());
    connection_data_ = NULL;
    return;
  }
  connection_data_ = std::make_shared<ConnectionParameters>();
  connection_data_->device_name = *device_name_ptr;

  /* Pull optional input channel. */
  const auto inp_channel_ptr = _get_token_value_int(params, "inp_channel");
  if (!inp_channel_ptr) {
    DEBUG("'inp_channel' parameter is ommited or misformed in '%s'. Using default 0.",
      params.c_str());
    /* 'inp_channel' parameter has been ommited. Use default input
     * channel, which is zero. */
    connection_data_->inp_channel = 0;
  } else {
    connection_data_->inp_channel = *inp_channel_ptr;
  }

  /* Increase socket buffer size to 4MB */
  // messenger->set_buffer_size(4194304);

  /* Get camera info for the emulated camera represented with this service.
   * Array of camera information records has been created when the camera
   * service was enumerating camera devices during the service initialization.
   * By the camera service protocol, camera service clients must first obtain
   * list of enumerated cameras via the 'list' query to the camera service, and
   * then use device name reported in the list to connect to an emulated camera
   * service. So, if camera information for the given device name is not found
   * in the array, we fail this connection due to protocol violation. */
  auto ci = CameraService::get()->getCameraInfoByDeviceName(connection_data_->device_name.c_str());
  if (!ci) {
    ERROR("Cannot find camera info for device '%s'",
      connection_data_->device_name.c_str());
    connection_data_ = NULL;
    return;
  }

  /* We can't allow multiple camera services for a single camera device, Lets
    * make sure that there is no client created for this camera. */
  if (ci->in_use) {
    ERROR("Camera device '%s' is in use", connection_data_->device_name.c_str());
    connection_data_ = NULL;
    return;
  }

  /* We're done. Set camera in use, and succeed the connection. */
  ci->in_use = true;
  connection_data_->camera_info = ci;

  DEBUG("Camera service is created for device '%s' using input channel %d",
    connection_data_->device_name.c_str(), connection_data_->inp_channel);
}

CameraMessageProcessor::~CameraMessageProcessor() {
  if (connection_data_) {
    DEBUG("Camera client for device '%s' on input channel %d is now closed",
      connection_data_->device_name.c_str(), connection_data_->inp_channel);
  }
}

bool CameraMessageProcessor::process_data(const std::vector<std::uint8_t> &data) {
  for (const auto &byte : data) {
    buffer_.push_back(byte);
  }

  process_commands();

  return true;
}

void CameraMessageProcessor::process_commands() {
  while (buffer_.size() > 0) {
    size_t size = 0;
    while (size < buffer_.size()) {
      if (buffer_.at(size) == 0x0) break;
      size++;
    }

    std::string command;
    command.insert(0, reinterpret_cast<const char *>(buffer_.data()), size);
    buffer_.erase(buffer_.begin(), buffer_.begin() + size + 1);

    handle_command(command);
  }
}

void CameraMessageProcessor::handle_command(const std::string &command) {
  const auto query = CameraQuery(command);

  if (query.query_name == "list") list();
  else if (query.query_name == "connect") connect(query.query_param);
  else if (query.query_name == "disconnect") disconnect(query.query_param);
  else if (query.query_name == "start") start(query.query_param);
  else if (query.query_name == "stop") stop(query.query_param);
  else if (query.query_name == "frame") frame(query.query_param);
  else {
    ERROR("%s: Unknown query '%s'", __FUNCTION__, query.query_name.c_str());
    reply_ko("Unknown query");
  }
}

void CameraMessageProcessor::list() {
  std::shared_ptr<CameraService> csd = CameraService::get();

  DEBUG("Camera service descriptor at %p", csd.get());

  /* Lets see if there was anything found... */
  if (csd->camera_info.size() == 0) {
    /* No cameras connected to the host. Reply with "\n" */
    DEBUG("No cameras connected.");
    reply_ok("\n");
    return;
  }

  std::string reply = "";

  /* "Stringify" each camera information into the reply string. */
  for (const auto ci : csd->camera_info) {
    reply += _camera_info_to_string(ci);
  }

  DEBUG("Replied: %s", reply.c_str());
  reply_ok(reply);
}

void CameraMessageProcessor::connect(const std::string &param) {
  UNUSED(param);

  if (!connection_data_) {
    ERROR("Unknown query 'connect'. Client is running in factory mode.");
    reply_ko("Unknown query 'connect'.");
    return;
  }

  if (connection_data_->camera) {
    /* Already connected. */
    WARNING("Camera '%s' is already connected", connection_data_->device_name.c_str());
    reply_ok("Camera is already connected");
    return;
  }

  /* Open camera device. */
  connection_data_->camera = 
      CameraDevice::openCameraDevice(connection_data_->device_name.c_str(),
                                     connection_data_->inp_channel);
  if (!connection_data_->camera) {
    ERROR("Unable to open camera device '%s'", connection_data_->device_name.c_str());
    reply_ko("Unable to open camera device.");
    return;
  }

  DEBUG("Camera device '%s' is now connected", connection_data_->device_name.c_str());

  reply_ok();
}

void CameraMessageProcessor::disconnect(const std::string &param) {
  UNUSED(param);

  if (!connection_data_) {
    ERROR("Unknown query 'disconnect'. Client is running in factory mode.");
    reply_ko("Unknown query 'disconnect'.");
    return;
  }

  if (connection_data_->camera == NULL) {
    /* Already disconnected. */
    WARNING("Camera '%s' is already disconnected",
            connection_data_->device_name.c_str());
    reply_ok("Camera is not connected");
    return;
  }

  /* Before we can go ahead and disconnect, we must make sure that camera is
   * not capturing frames. */
  if (connection_data_->video_frame != NULL) {
      ERROR("Cannot disconnect camera '%s' while it is not stopped",
            connection_data_->device_name.c_str());
      reply_ok("Camera is not stopped");
      return;
  }

  /* Close camera device. */
  connection_data_->camera->closeCameraDevice();
  connection_data_->camera = NULL;

  DEBUG("Camera device '%s' is now disconnected", connection_data_->device_name.c_str());

  reply_ok();
}

void CameraMessageProcessor::start(const std::string &param) {
  if (!connection_data_) {
    ERROR("Unknown query 'start'. Client is running in factory mode.");
    reply_ko("Unknown query 'start'.");
    return;
  }

  /* Sanity check. */
  if (connection_data_->camera == NULL) {
    /* Not connected. */
    ERROR("Camera '%s' is not connected", connection_data_->device_name.c_str());
    reply_ko("Camera is not connected");
    return;
  }

  /*
  * Parse parameters.
  */
  if (param.empty()) {
    ERROR("Missing parameters for the query");
    reply_ko("Missing parameters for the query");
    return;
  }

  /* Pull required 'dim' parameter. */
  auto dim_ptr = _get_token_value(param, "dim");
  if (!dim_ptr) {
    ERROR("Invalid or missing 'dim' parameter in '%s'", param.c_str());
    reply_ko("Invalid or missing 'dim' parameter");
    return;
  }
  std::string dim = *dim_ptr;

  /* Pull required 'pix' parameter. */
  auto pix_ptr = _get_token_value_int(param, "pix");
  if (!pix_ptr) {
    ERROR("Invalid or missing 'pix' parameter in '%s'", param.c_str());
    reply_ko("Invalid or missing 'pix' parameter");
    return;
  }
  long pix_format = *pix_ptr;

  /* Parse 'dim' parameter, and get requested frame width and height. */
  const auto dim_components = utils::string_split(dim, 'x');
  if (dim_components.size() != 2) {
    ERROR("Invalid 'dim' parameter in '%s'", param.c_str());
    reply_ko("Invalid 'dim' parameter");
    return;
  }
  errno = 0;
  long width = strtol(dim_components[0].c_str(), NULL, 10);
  long height = strtol(dim_components[1].c_str(), NULL, 10);
  if (errno) {
    ERROR("Invalid 'dim' parameter in '%s'", param.c_str());
    reply_ko("Invalid 'dim' parameter");
    return;
  }

  /* After collecting capture parameters lets see if camera has already
    * started, and if so, lets see if parameters match. */
  if (connection_data_->video_frame != NULL) {
    /* Already started. Match capture parameters. */
    if (connection_data_->pixel_format != pix_format ||connection_data_->width != width ||
      connection_data_->height != height) {
      /* Parameters match. Succeed the query. */
      WARNING("Camera '%s' is already started", connection_data_->device_name.c_str());
      reply_ok("Camera is already started");
    } else {
      /* Parameters don't match. Fail the query. */
      ERROR("Camera '%s' is already started, and parameters don't match:\n"
        "Current %.4s[%dx%d] != requested %.4s[%dx%d]",
        connection_data_->device_name, reinterpret_cast<const char*>(&connection_data_->pixel_format),
        connection_data_->width, connection_data_->height, reinterpret_cast<const char*>(&pix_format), width, height);
      reply_ko("Camera is already started with different capturing parameters");
    }
    return;
  }

  /* Start the camera.
   */

  /* Save capturing parameters. */
  connection_data_->pixel_format = pix_format;
  connection_data_->width = width;
  connection_data_->height = height;
  connection_data_->pixel_num = connection_data_->width * connection_data_->height;
  connection_data_->frames_cached = 0;

  /* Make sure that pixel format is known, and calculate video framebuffer size
   * along the lines. */
  switch (connection_data_->pixel_format) {
   case V4L2_PIX_FMT_YUV420:
   case V4L2_PIX_FMT_YVU420:
   case V4L2_PIX_FMT_NV12:
   case V4L2_PIX_FMT_NV21:
    connection_data_->video_frame_size = (connection_data_->pixel_num * 12) / 8;
    break;

   default:
    ERROR("Unknown pixel format %.4s",
      reinterpret_cast<char*>(&connection_data_->pixel_format));
    reply_ko("Pixel format is unknown");
    return;
  }

  /* Make sure that we have a converters between the original camera pixel
    * format and the one that the client expects. Also a converter must exist
    * for the preview window pixel format (RGB32) */
  if (!has_converter(connection_data_->camera_info->pixel_format, connection_data_->pixel_format) ||
      !has_converter(connection_data_->camera_info->pixel_format, V4L2_PIX_FMT_RGB32)) {
    ERROR("No conversion exist between %.4s and %.4s (or RGB32) pixel formats",
          reinterpret_cast<const char*>(&connection_data_->camera_info->pixel_format),
          reinterpret_cast<char*>(&connection_data_->pixel_format));
    reply_ko("No conversion exist for the requested pixel format");
    return;
  }

  /* TODO: At the moment camera framework in the emulator requires RGB32 pixel
   * format for preview window. So, we need to keep two framebuffers here: one
   * for the video, and another for the preview window. Watch out when this
   * changes (if changes). */
  connection_data_->preview_frame_size = connection_data_->pixel_num * 4;

  /* Allocate buffer large enough to contain both, video and preview
   * framebuffers. */
  connection_data_->video_frame =
      static_cast<uint8_t*>(malloc(connection_data_->video_frame_size + connection_data_->preview_frame_size));
  if (connection_data_->video_frame == NULL) {
    ERROR("Not enough memory for framebuffers %d + %d",
      connection_data_->video_frame_size, connection_data_->preview_frame_size);
    reply_ko("Out of memory");
    return;
  }

  /* Set framebuffer pointers. */
  connection_data_->preview_frame = reinterpret_cast<uint16_t*>(connection_data_->video_frame + connection_data_->video_frame_size);

  /* Start the camera. */
  if (connection_data_->camera->startCapturing(
       connection_data_->camera_info->pixel_format, connection_data_->width, connection_data_->height)) {
    ERROR("Cannot start camera '%s' for %.4s[%dx%d]: %s",
      connection_data_->device_name.c_str(),
      reinterpret_cast<const char*>(&connection_data_->pixel_format),
      connection_data_->width, connection_data_->height, strerror(errno));
    free(connection_data_->video_frame);
    connection_data_->video_frame = NULL;
    reply_ko("Cannot start the camera");
    return;
  }

  DEBUG("Camera '%s' is now started for %.4s[%dx%d]",
    connection_data_->device_name,
    reinterpret_cast<char*>(&connection_data_->pixel_format),
    connection_data_->width, connection_data_->height);

  reply_ok();
}

void CameraMessageProcessor::stop(const std::string &param) {
  UNUSED(param);

  if (!connection_data_) {
    ERROR("Unknown query 'stop'. Client is running in factory mode.");
    reply_ko("Unknown query 'stop'.");
    return;
  }

  if (connection_data_->video_frame == NULL) {
    /* Not started. */
    WARNING("%Camera '%s' is not started", connection_data_->device_name.c_str());
    reply_ko("Camera is not started");
    return;
  }

  /* Stop the camera. */
  if (connection_data_->camera->stopCapturing()) {
    ERROR("Cannot stop camera device '%s': %s",
      connection_data_->device_name.c_str(), strerror(errno));
    reply_ko("Cannot stop camera device");
    return;
  }

  free(connection_data_->video_frame);
  connection_data_->video_frame = NULL;

  DEBUG("Camera device '%s' is now stopped.", connection_data_->device_name.c_str());
  reply_ok();
}

void CameraMessageProcessor::frame(const std::string &param) {

  size_t video_size = 0;
  size_t preview_size = 0;
  int repeat;
  anbox::camera::ClientFrameBuffer fbs[2];
  int fbs_num = 0;
  size_t payload_size;
  uint64_t tick;
  float r_scale = 1.0f, g_scale = 1.0f, b_scale = 1.0f, exp_comp = 1.0f;

  /* Sanity checks */
  if (!connection_data_) {
    ERROR("Unknown query 'frame'. Client is running in factory mode.");
    reply_ko("Unknown query 'frame'.");
    return;
  }
  if (connection_data_->video_frame == NULL) {
    /* Not started. */
    ERROR("Camera '%s' is not started", connection_data_->device_name);
    reply_ko("Camera is not started");
    return;
  }

  /* Pull required parameters. */
  const auto video_size_ptr = _get_token_value_int(param, "video");
  const auto preview_size_ptr = _get_token_value_int(param, "preview");
  if (!video_size_ptr || !preview_size_ptr) {
    ERROR("Invalid or missing 'video', or 'preview' parameter in '%s'", param);
    reply_ko("Invalid or missing 'video', or 'preview' parameter");
    return;
  }
  video_size = *video_size_ptr;
  preview_size = *preview_size_ptr;

  /* Pull white balance values. */
  auto tmp = _get_token_value(param, "whiteb");
  if (tmp) {
    if (sscanf(tmp->c_str(), "%g,%g,%g", &r_scale, &g_scale, &b_scale) != 3) {
      DEBUG("Invalid value '%s' for parameter 'whiteb'", tmp->c_str());
      r_scale = g_scale = b_scale = 1.0f;
    }
  }

  /* Pull exposure compensation. */
  tmp = _get_token_value(param, "expcomp");
  if (tmp) {
    if (sscanf(tmp->c_str(), "%g", &exp_comp) != 1) {
      DEBUG("Invalid value '%s' for parameter 'whiteb'", tmp->c_str());
      exp_comp = 1.0f;
    }
  }

  /* Verify that framebuffer sizes match the ones that the started camera
   * operates with. */
  if ((video_size != 0 && connection_data_->video_frame_size != video_size) ||
      (preview_size != 0 && connection_data_->preview_frame_size != preview_size)) {
    ERROR("Frame sizes don't match for camera '%s':\n"
      "Expected %d for video, and %d for preview. Requested %d, and %d",
      connection_data_->device_name, connection_data_->video_frame_size,
      connection_data_->preview_frame_size, video_size, preview_size);
    reply_ko("Frame size mismatch");
    return;
  }

  /*
   * Initialize framebuffer array for frame read.
   */

  if (video_size) {
    fbs[fbs_num].pixel_format = connection_data_->pixel_format;
    fbs[fbs_num].framebuffer = connection_data_->video_frame;
    fbs_num++;
  }
  if (preview_size) {
    /* TODO: Watch out for preview format changes! */
    fbs[fbs_num].pixel_format = V4L2_PIX_FMT_RGB32;
    fbs[fbs_num].framebuffer = connection_data_->preview_frame;
    fbs_num++;
  }

  /* Capture new frame. */
  tick = anbox::camera::_get_timestamp();
  repeat = connection_data_->camera->readFrame(fbs, fbs_num,
                                    r_scale, g_scale, b_scale, exp_comp);

  /* Note that there is no (known) way how to wait on next frame being
   * available, so we could dequeue frame buffer from the device only when we
   * know it's available. Instead we're shooting in the dark, and quite often
   * device will response with EAGAIN, indicating that it doesn't have frame
   * ready. In turn, it means that the last frame we have obtained from the
   * device is still good, and we can reply with the cached frames. The only
   * case when we need to keep trying to obtain a new frame is when frame cache
   * is empty. To prevent ourselves from an indefinite loop in case device got
   * stuck on something (observed with some Microsoft devices) we will limit
   * the loop by 2 second time period (which is more than enough to obtain
   * something from the device) */
  while (repeat == 1 && !connection_data_->frames_cached &&
         (anbox::camera::_get_timestamp() - tick) < 2000000LL) {
    /* Sleep for 1 millisec before repeating the attempt. */
    anbox::camera::_camera_sleep(1);
    repeat = connection_data_->camera->readFrame(fbs, fbs_num,
                                      r_scale, g_scale, b_scale, exp_comp);
  }
  if (repeat == 1 && !connection_data_->frames_cached) {
    /* Waited too long for the first frame. */
    ERROR("Unable to obtain first video frame from the camera '%s' in %d milliseconds: %s.",
      connection_data_->device_name,
      static_cast<uint32_t>(anbox::camera::_get_timestamp() - tick) / 1000, strerror(errno));
    reply_ko("Unable to obtain video frame from the camera");
    return;
  } else if (repeat < 0) {
    /* An I/O error. */
    ERROR("Unable to obtain video frame from the camera '%s': %s.",
      connection_data_->device_name, strerror(errno));
    reply_ko(strerror(errno));
    return;
  }

  /* We have cached something... */
  connection_data_->frames_cached = 1;

  /* Build the reply.
   */

  /* Payload includes "ok:" + requested video and preview frames. */
  payload_size = 3 + video_size + preview_size;

  /* Send payload size first. */
  send_header(payload_size);

  /* After that send the 'ok:'. Note that if there is no frames sent, we should
   * use prefix "ok" instead of "ok:" */
  if (video_size || preview_size) {
    send_bytes("ok:", 3);
  } else {
    /* Still 3 bytes: zero terminator is required in this case. */
    send_bytes("ok", 3);
  }

  static_assert(sizeof(char) == sizeof(uint8_t), "Must be on an 8-bit system!");

  /* After that send video frame (if requested). */
  if (video_size) {
      send_bytes(reinterpret_cast<const char*>(connection_data_->video_frame), video_size);
  }

  /* After that send preview frame (if requested). */
  if (preview_size) {
      send_bytes(reinterpret_cast<const char*>(connection_data_->preview_frame), preview_size);
  }
}

/* Builds and sends a reply to a query.
 * All replies to a query in camera service have a prefix indicating whether the
 * query has succeeded ("ok"), or failed ("ko"). The prefix can be followed by
 * extra data, containing response to the query. In case there are extra data,
 * they are separated from the prefix with a ':' character.
 * Param:
 *  qc - Qemu client to send the reply to.
 *  ok_ko - An "ok", or "ko" selector, where 0 is for "ko", and !0 is for "ok".
 *  extra - Optional extra query data. Can be NULL.
 *  extra_size - Extra data size.
 */
void CameraMessageProcessor::query_reply(bool ok_ko,
                                         const void* extra,
                                         size_t extra_size) {
  const char* ok_ko_str;
  size_t payload_size;

  /* Make sure extra_size is 0 if extra is NULL. */
  if (extra == NULL && extra_size != 0) {
    WARNING("'extra' = NULL, while 'extra_size' = %d",
      static_cast<int>(extra_size));
    extra_size = 0;
  }

  /* Calculate total payload size, and select appropriate 'ok'/'ko' prefix */
  if (extra_size) {
    /* 'extra' size + 2 'ok'/'ko' bytes + 1 ':' separator byte. */
    payload_size = extra_size + 3;
    ok_ko_str = ok_ko ? OK_REPLY_DATA : KO_REPLY_DATA;
  } else {
    /* No extra data: just zero-terminated 'ok'/'ko'. */
    payload_size = 3;
    ok_ko_str = ok_ko ? OK_REPLY : KO_REPLY;
  }

  /* Send payload size first. */
  send_header(payload_size);
  /* Send 'ok[:]'/'ko[:]' next. Note that if there is no extra data, we still
   * need to send a zero-terminator for 'ok'/'ko' string instead of the ':'
   * separator. So, one way or another, the prefix is always 3 bytes. */
  send_bytes(ok_ko_str, 3);
  /* Send extra data (if present). */
  if (extra != NULL) {
      send_bytes(reinterpret_cast<const char*>(extra), extra_size);
  }
}

void CameraMessageProcessor::reply_ok() {
  query_reply(true, NULL, 0);
}

void CameraMessageProcessor::reply_ok(const std::string &result_str) {
  query_reply(true, result_str.c_str(), result_str.size());
}

void CameraMessageProcessor::reply_ko(const std::string &result_str) {
  query_reply(false, result_str.c_str(), result_str.size());
}

static const int header_size = 8;

void CameraMessageProcessor::send_header(const size_t &size) {
  char header[header_size + 1];
  std::snprintf(header, header_size + 1, "%08zx", size);
  send_bytes(header, header_size);
}

void CameraMessageProcessor::send_bytes(const char *msg, size_t size) {
  size_t bytes_written = 0;
  while (bytes_written < size) {
    int result = messenger_->send_raw(msg + bytes_written, size - bytes_written);
    if (result <= 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // Reader is too slow
      } else {
        ERROR("Failed to send %d bytes: %s", size - bytes_written, strerror(errno));
        break;
      }
    } else {
      bytes_written += result;
    }
  }
}
}