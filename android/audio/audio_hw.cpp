/*
 * Copyright (C) 2012 The Android Open Source Project
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
 */

#define LOG_TAG "audio_hw_generic"
#define LOG_NDEBUG 0

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cutils/log.h>
#include <cutils/str_parms.h>

#include <hardware/audio.h>
#include <hardware/hardware.h>
#include <system/audio.h>

#include "anbox/audio/client_info.h"

#define AUDIO_DEVICE_NAME "/dev/anbox_audio"
#define OUT_SAMPLING_RATE 44100
#define OUT_BUFFER_SIZE 4096
#define OUT_LATENCY_MS 20
#define IN_SAMPLING_RATE 8000
#define IN_BUFFER_SIZE 320

struct generic_audio_device {
  struct audio_hw_device device;
  pthread_mutex_t lock;
  struct audio_stream_out *output;
  struct audio_stream_in *input;
  bool mic_mute;
};

struct generic_stream_out {
  struct audio_stream_out stream;
  struct generic_audio_device *dev;
  audio_devices_t device;
  int fd;
};

struct generic_stream_in {
  struct audio_stream_in stream;
  struct generic_audio_device *dev;
  audio_devices_t device;
  int fd;
};

static uint32_t out_get_sample_rate(const struct audio_stream *stream) {
  return OUT_SAMPLING_RATE;
}

static int out_set_sample_rate(struct audio_stream *stream, uint32_t rate) {
  return -ENOSYS;
}

static size_t out_get_buffer_size(const struct audio_stream *stream) {
  return OUT_BUFFER_SIZE;
}

static audio_channel_mask_t out_get_channels(const struct audio_stream *stream) {
  return AUDIO_CHANNEL_OUT_STEREO;
}

static audio_format_t out_get_format(const struct audio_stream *stream) {
  return AUDIO_FORMAT_PCM_16_BIT;
}

static int out_set_format(struct audio_stream *stream, audio_format_t format) {
  return -ENOSYS;
}

static int out_standby(struct audio_stream *stream) {
  return 0;
}

static int out_dump(const struct audio_stream *stream, int fd) {
  struct generic_stream_out *out = (struct generic_stream_out *)stream;

  dprintf(fd,
          "\tout_dump:\n"
          "\t\tsample rate: %u\n"
          "\t\tbuffer size: %u\n"
          "\t\tchannel mask: %08x\n"
          "\t\tformat: %d\n"
          "\t\tdevice: %08x\n"
          "\t\taudio dev: %p\n\n",
          out_get_sample_rate(stream),
          out_get_buffer_size(stream),
          out_get_channels(stream),
          out_get_format(stream),
          out->device,
          out->dev);

  return 0;
}

static int out_set_parameters(struct audio_stream *stream, const char *kvpairs) {
  struct generic_stream_out *out = (struct generic_stream_out *)stream;
  struct str_parms *parms;
  char value[32];
  int ret;
  long val;
  char *end;

  parms = str_parms_create_str(kvpairs);

  ret = str_parms_get_str(parms, AUDIO_PARAMETER_STREAM_ROUTING,
                          value, sizeof(value));
  if (ret >= 0) {
    errno = 0;
    val = strtol(value, &end, 10);
    if (errno == 0 && (end != NULL) && (*end == '\0') && ((int)val == val)) {
      out->device = (int)val;
    } else {
      ret = -EINVAL;
    }
  }

  str_parms_destroy(parms);
  return ret;
}

static char *out_get_parameters(const struct audio_stream *stream, const char *keys) {
  struct generic_stream_out *out = (struct generic_stream_out *)stream;
  struct str_parms *query = str_parms_create_str(keys);
  char *str;
  char value[256];
  struct str_parms *reply = str_parms_create();
  int ret;

  ret = str_parms_get_str(query, AUDIO_PARAMETER_STREAM_ROUTING, value, sizeof(value));
  if (ret >= 0) {
    str_parms_add_int(reply, AUDIO_PARAMETER_STREAM_ROUTING, out->device);
    str = strdup(str_parms_to_str(reply));
  } else {
    str = strdup(keys);
  }

  str_parms_destroy(query);
  str_parms_destroy(reply);
  return str;
}

static uint32_t out_get_latency(const struct audio_stream_out *stream) {
  return OUT_LATENCY_MS;
}

static int out_set_volume(struct audio_stream_out *stream, float left,
                          float right) {
  return -ENOSYS;
}

static ssize_t out_write(struct audio_stream_out *stream, const void *buffer,
                         size_t bytes) {
  struct generic_stream_out *out = (struct generic_stream_out *)stream;
  struct generic_audio_device *adev = out->dev;

  pthread_mutex_lock(&adev->lock);
  if (out->fd >= 0)
    bytes = write(out->fd, buffer, bytes);

  // wait until session writes the data we sent,
  // this will block if sink queue is full,
  // acting as synchronization to time audio
  int64_t arrived_us;
  read(out->fd, &arrived_us, sizeof(arrived_us));
  (void) arrived_us;

  pthread_mutex_unlock(&adev->lock);
  return bytes;
}

static int out_get_render_position(const struct audio_stream_out *stream,
                                   uint32_t *dsp_frames) {
  return -ENOSYS;
}

static int out_add_audio_effect(const struct audio_stream *stream, effect_handle_t effect) {
  return 0;
}

static int out_remove_audio_effect(const struct audio_stream *stream, effect_handle_t effect) {
  return 0;
}

static int out_get_next_write_timestamp(const struct audio_stream_out *stream,
                                        int64_t *timestamp) {
  return -ENOSYS;
}

static uint32_t in_get_sample_rate(const struct audio_stream *stream) {
  return IN_SAMPLING_RATE;
}

static int in_set_sample_rate(struct audio_stream *stream, uint32_t rate) {
  return -ENOSYS;
}

static size_t in_get_buffer_size(const struct audio_stream *stream) {
  return IN_BUFFER_SIZE;
}

static audio_channel_mask_t in_get_channels(const struct audio_stream *stream) {
  return AUDIO_CHANNEL_IN_MONO;
}

static audio_format_t in_get_format(const struct audio_stream *stream) {
  return AUDIO_FORMAT_PCM_16_BIT;
}

static int in_set_format(struct audio_stream *stream, audio_format_t format) {
  return -ENOSYS;
}

static int in_standby(struct audio_stream *stream) {
  return 0;
}

static int in_dump(const struct audio_stream *stream, int fd) {
  struct generic_stream_in *in = (struct generic_stream_in *)stream;

  dprintf(fd,
          "\tin_dump:\n"
          "\t\tsample rate: %u\n"
          "\t\tbuffer size: %u\n"
          "\t\tchannel mask: %08x\n"
          "\t\tformat: %d\n"
          "\t\tdevice: %08x\n"
          "\t\taudio dev: %p\n\n",
          in_get_sample_rate(stream),
          in_get_buffer_size(stream),
          in_get_channels(stream),
          in_get_format(stream),
          in->device,
          in->dev);

  return 0;
}

static int in_set_parameters(struct audio_stream *stream, const char *kvpairs) {
  struct generic_stream_in *in = (struct generic_stream_in *)stream;
  struct str_parms *parms;
  char value[32];
  int ret;
  long val;
  char *end;

  parms = str_parms_create_str(kvpairs);

  ret = str_parms_get_str(parms, AUDIO_PARAMETER_STREAM_ROUTING,
                          value, sizeof(value));
  if (ret >= 0) {
    errno = 0;
    val = strtol(value, &end, 10);
    if ((errno == 0) && (end != NULL) && (*end == '\0') && ((int)val == val)) {
      in->device = (int)val;
    } else {
      ret = -EINVAL;
    }
  }

  str_parms_destroy(parms);
  return ret;
}

static char *in_get_parameters(const struct audio_stream *stream,
                               const char *keys) {
  struct generic_stream_in *in = (struct generic_stream_in *)stream;
  struct str_parms *query = str_parms_create_str(keys);
  char *str;
  char value[256];
  struct str_parms *reply = str_parms_create();
  int ret;

  ret = str_parms_get_str(query, AUDIO_PARAMETER_STREAM_ROUTING, value, sizeof(value));
  if (ret >= 0) {
    str_parms_add_int(reply, AUDIO_PARAMETER_STREAM_ROUTING, in->device);
    str = strdup(str_parms_to_str(reply));
  } else {
    str = strdup(keys);
  }

  str_parms_destroy(query);
  str_parms_destroy(reply);
  return str;
}

static int in_set_gain(struct audio_stream_in *stream, float gain) {
  return 0;
}

static ssize_t in_read(struct audio_stream_in *stream, void *buffer,
                       size_t bytes) {
  struct generic_stream_in *in = (struct generic_stream_in *)stream;
  struct generic_audio_device *adev = in->dev;

  pthread_mutex_lock(&adev->lock);
  if (in->fd >= 0)
    bytes = read(in->fd, buffer, bytes);
  if (adev->mic_mute && (bytes > 0)) {
    memset(buffer, 0, bytes);
  }
  pthread_mutex_unlock(&adev->lock);

  return bytes;
}

static uint32_t in_get_input_frames_lost(struct audio_stream_in *stream) {
  return 0;
}

static int in_add_audio_effect(const struct audio_stream *stream, effect_handle_t effect) {
  return 0;
}

static int in_remove_audio_effect(const struct audio_stream *stream, effect_handle_t effect) {
  return 0;
}

static int connect_audio_server(const anbox::audio::ClientInfo::Type &type) {
  int fd = socket(AF_LOCAL, SOCK_STREAM, 0);
  if (fd < 0)
    return -errno;

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, AUDIO_DEVICE_NAME, sizeof(addr.sun_path));

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(fd);
    return -errno;
  }

  // We will send out client type information to the server and the
  // server will either deny the request by closing the connection
  // or by sending us the approved client details back.
  anbox::audio::ClientInfo client_info{type};
  if (::write(fd, &client_info, sizeof(client_info)) < 0) {
    close(fd);
    return -EIO;
  }

  auto bytes_read = ::read(fd, &client_info, sizeof(client_info));
  if (bytes_read < 0) {
    close(fd);
    return -EIO;
  }

  // FIXME once we have real client details we need to check if we
  // got everything we need or if anything is missing.

  ALOGE("Successfully connected Anbox audio server");

  return fd;
}

static int adev_open_output_stream(struct audio_hw_device *dev,
                                   audio_io_handle_t handle,
                                   audio_devices_t devices,
                                   audio_output_flags_t flags,
                                   struct audio_config *config,
                                   struct audio_stream_out **stream_out,
                                   const char *address __unused) {
  struct generic_audio_device *adev = (struct generic_audio_device *)dev;
  struct generic_stream_out *out;
  int ret = 0, fd = 0;

  pthread_mutex_lock(&adev->lock);
  if (adev->output != NULL) {
    ret = -ENOSYS;
    goto error;
  }

  fd = connect_audio_server(anbox::audio::ClientInfo::Type::Playback);
  if (fd < 0) {
    ret = fd;
    ALOGE("Failed to connect with Anbox audio servers (err %d)", ret);
    goto error;
  }

  if ((config->format != AUDIO_FORMAT_PCM_16_BIT) ||
      (config->channel_mask != AUDIO_CHANNEL_OUT_STEREO) ||
      (config->sample_rate != OUT_SAMPLING_RATE)) {
    ALOGE("Error opening output stream format %d, channel_mask %04x, sample_rate %u",
          config->format, config->channel_mask, config->sample_rate);
    config->format = AUDIO_FORMAT_PCM_16_BIT;
    config->channel_mask = AUDIO_CHANNEL_OUT_STEREO;
    config->sample_rate = OUT_SAMPLING_RATE;
    ret = -EINVAL;
    goto error;
  }

  out = (struct generic_stream_out *)calloc(1, sizeof(struct generic_stream_out));
  out->fd = fd;

  out->stream.common.get_sample_rate = out_get_sample_rate;
  out->stream.common.set_sample_rate = out_set_sample_rate;
  out->stream.common.get_buffer_size = out_get_buffer_size;
  out->stream.common.get_channels = out_get_channels;
  out->stream.common.get_format = out_get_format;
  out->stream.common.set_format = out_set_format;
  out->stream.common.standby = out_standby;
  out->stream.common.dump = out_dump;
  out->stream.common.set_parameters = out_set_parameters;
  out->stream.common.get_parameters = out_get_parameters;
  out->stream.common.add_audio_effect = out_add_audio_effect;
  out->stream.common.remove_audio_effect = out_remove_audio_effect;
  out->stream.get_latency = out_get_latency;
  out->stream.set_volume = out_set_volume;
  out->stream.write = out_write;
  out->stream.get_render_position = out_get_render_position;
  out->stream.get_next_write_timestamp = out_get_next_write_timestamp;

  out->dev = adev;
  out->device = devices;
  adev->output = (struct audio_stream_out *)out;
  *stream_out = &out->stream;

error:
  pthread_mutex_unlock(&adev->lock);

  return ret;
}

static void adev_close_output_stream(struct audio_hw_device *dev,
                                     struct audio_stream_out *stream) {
  struct generic_audio_device *adev = (struct generic_audio_device *)dev;

  pthread_mutex_lock(&adev->lock);
  if (stream == adev->output) {
    free(stream);
    adev->output = NULL;
  }
  pthread_mutex_unlock(&adev->lock);
}

static int adev_set_parameters(struct audio_hw_device *dev, const char *kvpairs) {
  return 0;
}

static char *adev_get_parameters(const struct audio_hw_device *dev,
                                 const char *keys) {
  return strdup("");
}

static int adev_init_check(const struct audio_hw_device *dev) {
  return 0;
}

static int adev_set_voice_volume(struct audio_hw_device *dev, float volume) {
  return 0;
}

static int adev_set_master_volume(struct audio_hw_device *dev, float volume) {
  return -ENOSYS;
}

static int adev_get_master_volume(struct audio_hw_device *dev, float *volume) {
  return -ENOSYS;
}

static int adev_set_master_mute(struct audio_hw_device *dev, bool muted) {
  return -ENOSYS;
}

static int adev_get_master_mute(struct audio_hw_device *dev, bool *muted) {
  return -ENOSYS;
}

static int adev_set_mode(struct audio_hw_device *dev, audio_mode_t mode) {
  return 0;
}

static int adev_set_mic_mute(struct audio_hw_device *dev, bool state) {
  struct generic_audio_device *adev = (struct generic_audio_device *)dev;

  pthread_mutex_lock(&adev->lock);
  adev->mic_mute = state;
  pthread_mutex_unlock(&adev->lock);
  return 0;
}

static int adev_get_mic_mute(const struct audio_hw_device *dev, bool *state) {
  struct generic_audio_device *adev = (struct generic_audio_device *)dev;

  pthread_mutex_lock(&adev->lock);
  *state = adev->mic_mute;
  pthread_mutex_unlock(&adev->lock);

  return 0;
}

static size_t adev_get_input_buffer_size(const struct audio_hw_device *dev,
                                         const struct audio_config *config) {
  return IN_BUFFER_SIZE;
}

static int adev_open_input_stream(struct audio_hw_device *dev,
                                  audio_io_handle_t handle,
                                  audio_devices_t devices,
                                  struct audio_config *config,
                                  struct audio_stream_in **stream_in,
                                  audio_input_flags_t flags __unused,
                                  const char *address __unused,
                                  audio_source_t source __unused) {
  struct generic_audio_device *adev = (struct generic_audio_device *)dev;
  struct generic_stream_in *in;
  int ret = 0, fd = 0;

  pthread_mutex_lock(&adev->lock);
  if (adev->input != NULL) {
    ret = -ENOSYS;
    goto error;
  }

  if ((config->format != AUDIO_FORMAT_PCM_16_BIT) ||
      (config->channel_mask != AUDIO_CHANNEL_IN_MONO) ||
      (config->sample_rate != IN_SAMPLING_RATE)) {
    ALOGE("Error opening input stream format %d, channel_mask %04x, sample_rate %u",
          config->format, config->channel_mask, config->sample_rate);
    config->format = AUDIO_FORMAT_PCM_16_BIT;
    config->channel_mask = AUDIO_CHANNEL_IN_MONO;
    config->sample_rate = IN_SAMPLING_RATE;
    ret = -EINVAL;
    goto error;
  }

  fd = connect_audio_server(anbox::audio::ClientInfo::Type::Recording);
  if (fd < 0) {
    ret = fd;
    ALOGE("Failed to connect with Anbox audio servers (err %d)", ret);
    goto error;
  }

  in = (struct generic_stream_in *)calloc(1, sizeof(struct generic_stream_in));
  in->fd = fd;

  in->stream.common.get_sample_rate = in_get_sample_rate;
  in->stream.common.set_sample_rate = in_set_sample_rate;
  in->stream.common.get_buffer_size = in_get_buffer_size;
  in->stream.common.get_channels = in_get_channels;
  in->stream.common.get_format = in_get_format;
  in->stream.common.set_format = in_set_format;
  in->stream.common.standby = in_standby;
  in->stream.common.dump = in_dump;
  in->stream.common.set_parameters = in_set_parameters;
  in->stream.common.get_parameters = in_get_parameters;
  in->stream.common.add_audio_effect = in_add_audio_effect;
  in->stream.common.remove_audio_effect = in_remove_audio_effect;
  in->stream.set_gain = in_set_gain;
  in->stream.read = in_read;
  in->stream.get_input_frames_lost = in_get_input_frames_lost;

  in->dev = adev;
  in->device = devices;
  adev->input = (struct audio_stream_in *)in;
  *stream_in = &in->stream;

error:
  pthread_mutex_unlock(&adev->lock);

  return ret;
}

static void adev_close_input_stream(struct audio_hw_device *dev,
                                    struct audio_stream_in *stream) {
  struct generic_audio_device *adev = (struct generic_audio_device *)dev;

  pthread_mutex_lock(&adev->lock);
  if (stream == adev->input) {
    free(stream);
    adev->input = NULL;
  }
  pthread_mutex_unlock(&adev->lock);
}

static int adev_dump(const audio_hw_device_t *dev, int fd) {
  struct generic_audio_device *adev = (struct generic_audio_device *)dev;

  const size_t SIZE = 256;
  char buffer[SIZE];

  dprintf(fd,
          "\nadev_dump:\n"
          "\tmic_mute: %s\n"
          "\toutput: %p\n"
          "\tinput: %p\n\n",
          adev->mic_mute ? "true" : "false",
          adev->output,
          adev->input);

  if (adev->output != NULL)
    out_dump((const struct audio_stream *)adev->output, fd);
  if (adev->input != NULL)
    in_dump((const struct audio_stream *)adev->input, fd);

  return 0;
}

static int adev_close(hw_device_t *dev) {
  struct generic_audio_device *adev = (struct generic_audio_device *)dev;

  adev_close_output_stream((struct audio_hw_device *)dev, adev->output);
  adev_close_input_stream((struct audio_hw_device *)dev, adev->input);

  free(dev);
  return 0;
}

static int adev_open(const hw_module_t *module, const char *name,
                     hw_device_t **device) {
  struct generic_audio_device *adev;

  if (strcmp(name, AUDIO_HARDWARE_INTERFACE) != 0)
    return -EINVAL;

  adev = (struct generic_audio_device*) calloc(1, sizeof(struct generic_audio_device));

  adev->device.common.tag = HARDWARE_DEVICE_TAG;
  adev->device.common.version = AUDIO_DEVICE_API_VERSION_2_0;
  adev->device.common.module = (struct hw_module_t *)module;
  adev->device.common.close = adev_close;

  adev->device.init_check = adev_init_check;
  adev->device.set_voice_volume = adev_set_voice_volume;
  adev->device.set_master_volume = adev_set_master_volume;
  adev->device.get_master_volume = adev_get_master_volume;
  adev->device.set_master_mute = adev_set_master_mute;
  adev->device.get_master_mute = adev_get_master_mute;
  adev->device.set_mode = adev_set_mode;
  adev->device.set_mic_mute = adev_set_mic_mute;
  adev->device.get_mic_mute = adev_get_mic_mute;
  adev->device.set_parameters = adev_set_parameters;
  adev->device.get_parameters = adev_get_parameters;
  adev->device.get_input_buffer_size = adev_get_input_buffer_size;
  adev->device.open_output_stream = adev_open_output_stream;
  adev->device.close_output_stream = adev_close_output_stream;
  adev->device.open_input_stream = adev_open_input_stream;
  adev->device.close_input_stream = adev_close_input_stream;
  adev->device.dump = adev_dump;

  *device = &adev->device.common;

  return 0;
}

static struct hw_module_methods_t hal_module_methods = {
    .open = adev_open,
};

struct audio_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = AUDIO_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = AUDIO_HARDWARE_MODULE_ID,
        .name = "Anbox audio HW HAL",
        .author = "The Android Open Source Project",
        .methods = &hal_module_methods,
    },
};
