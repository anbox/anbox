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
 */

/*
 * Contains implementation of classes that encapsulate connection to camera
 * services in the emulator via qemu pipe.
 */

#define LOG_NDEBUG 1
#define LOG_TAG "EmulatedCamera_QemuClient"
#include <cutils/log.h>
#include "EmulatedCamera.h"
#include "QemuClient.h"

#define LOG_QUERIES 0
#if LOG_QUERIES
#define LOGQ(...)   ALOGD(__VA_ARGS__)
#else
#define LOGQ(...)   (void(0))

#endif  // LOG_QUERIES
namespace android {

/****************************************************************************
 * Qemu query
 ***************************************************************************/

QemuQuery::QemuQuery()
    : mQuery(mQueryPrealloc),
      mQueryDeliveryStatus(NO_ERROR),
      mReplyBuffer(NULL),
      mReplyData(NULL),
      mReplySize(0),
      mReplyDataSize(0),
      mReplyStatus(0)
{
    *mQuery = '\0';
}

QemuQuery::QemuQuery(const char* query_string)
    : mQuery(mQueryPrealloc),
      mQueryDeliveryStatus(NO_ERROR),
      mReplyBuffer(NULL),
      mReplyData(NULL),
      mReplySize(0),
      mReplyDataSize(0),
      mReplyStatus(0)
{
    mQueryDeliveryStatus = QemuQuery::createQuery(query_string, NULL);
}

QemuQuery::QemuQuery(const char* query_name, const char* query_param)
    : mQuery(mQueryPrealloc),
      mQueryDeliveryStatus(NO_ERROR),
      mReplyBuffer(NULL),
      mReplyData(NULL),
      mReplySize(0),
      mReplyDataSize(0),
      mReplyStatus(0)
{
    mQueryDeliveryStatus = QemuQuery::createQuery(query_name, query_param);
}

QemuQuery::~QemuQuery()
{
    QemuQuery::resetQuery();
}

status_t QemuQuery::createQuery(const char* name, const char* param)
{
    /* Reset from the previous use. */
    resetQuery();

    /* Query name cannot be NULL or an empty string. */
    if (name == NULL || *name == '\0') {
        ALOGE("%s: NULL or an empty string is passed as query name.",
             __FUNCTION__);
        mQueryDeliveryStatus = EINVAL;
        return EINVAL;
    }

    const size_t name_len = strlen(name);
    const size_t param_len = (param != NULL) ? strlen(param) : 0;
    const size_t required = strlen(name) + (param_len ? (param_len + 2) : 1);

    if (required > sizeof(mQueryPrealloc)) {
        /* Preallocated buffer was too small. Allocate a bigger query buffer. */
        mQuery = new char[required];
        if (mQuery == NULL) {
            ALOGE("%s: Unable to allocate %zu bytes for query buffer",
                 __FUNCTION__, required);
            mQueryDeliveryStatus = ENOMEM;
            return ENOMEM;
        }
    }

    /* At this point mQuery buffer is big enough for the query. */
    if (param_len) {
        sprintf(mQuery, "%s %s", name, param);
    } else {
        memcpy(mQuery, name, name_len + 1);
    }

    return NO_ERROR;
}

status_t QemuQuery::completeQuery(status_t status)
{
    /* Save query completion status. */
    mQueryDeliveryStatus = status;
    if (mQueryDeliveryStatus != NO_ERROR) {
        return mQueryDeliveryStatus;
    }

    /* Make sure reply buffer contains at least 'ok', or 'ko'.
     * Note that 'ok', or 'ko' prefixes are always 3 characters long: in case
     * there are more data in the reply, that data will be separated from 'ok'/'ko'
     * with a ':'. If there is no more data in the reply, the prefix will be
     * zero-terminated, and the terminator will be inculded in the reply. */
    if (mReplyBuffer == NULL || mReplySize < 3) {
        ALOGE("%s: Invalid reply to the query", __FUNCTION__);
        mQueryDeliveryStatus = EINVAL;
        return EINVAL;
    }

    /* Lets see the reply status. */
    if (!memcmp(mReplyBuffer, "ok", 2)) {
        mReplyStatus = 1;
    } else if (!memcmp(mReplyBuffer, "ko", 2)) {
        mReplyStatus = 0;
    } else {
        ALOGE("%s: Invalid query reply: '%s'", __FUNCTION__, mReplyBuffer);
        mQueryDeliveryStatus = EINVAL;
        return EINVAL;
    }

    /* Lets see if there are reply data that follow. */
    if (mReplySize > 3) {
        /* There are extra data. Make sure they are separated from the status
         * with a ':' */
        if (mReplyBuffer[2] != ':') {
            ALOGE("%s: Invalid query reply: '%s'", __FUNCTION__, mReplyBuffer);
            mQueryDeliveryStatus = EINVAL;
            return EINVAL;
        }
        mReplyData = mReplyBuffer + 3;
        mReplyDataSize = mReplySize - 3;
    } else {
        /* Make sure reply buffer containing just 'ok'/'ko' ends with
         * zero-terminator. */
        if (mReplyBuffer[2] != '\0') {
            ALOGE("%s: Invalid query reply: '%s'", __FUNCTION__, mReplyBuffer);
            mQueryDeliveryStatus = EINVAL;
            return EINVAL;
        }
    }

    return NO_ERROR;
}

void QemuQuery::resetQuery()
{
    if (mQuery != NULL && mQuery != mQueryPrealloc) {
        delete[] mQuery;
    }
    mQuery = mQueryPrealloc;
    mQueryDeliveryStatus = NO_ERROR;
    if (mReplyBuffer != NULL) {
        free(mReplyBuffer);
        mReplyBuffer = NULL;
    }
    mReplyData = NULL;
    mReplySize = mReplyDataSize = 0;
    mReplyStatus = 0;
}

/****************************************************************************
 * Qemu client base
 ***************************************************************************/

/* Camera service name. */
const char QemuClient::mCameraServiceName[]   = "camera";

QemuClient::QemuClient()
    : mPipeFD(-1)
{
}

QemuClient::~QemuClient()
{
    if (mPipeFD >= 0) {
        close(mPipeFD);
    }
}

/****************************************************************************
 * Qemu client API
 ***************************************************************************/

status_t QemuClient::connectClient(const char* param)
{
    ALOGV("%s: '%s'", __FUNCTION__, param ? param : "");

    /* Make sure that client is not connected already. */
    if (mPipeFD >= 0) {
        ALOGE("%s: Qemu client is already connected", __FUNCTION__);
        return EINVAL;
    }

    /* Select one of the two: 'factory', or 'emulated camera' service */
    if (param == NULL || *param == '\0') {
        /* No parameters: connect to the factory service. */
        char pipe_name[512];
        snprintf(pipe_name, sizeof(pipe_name), "qemud:%s", mCameraServiceName);
        mPipeFD = qemu_pipe_open(pipe_name);
    } else {
        /* One extra char ':' that separates service name and parameters + six
         * characters for 'qemud:'. This is required by qemu pipe protocol. */
        char* connection_str = new char[strlen(mCameraServiceName) +
                                        strlen(param) + 8];
        sprintf(connection_str, "qemud:%s:%s", mCameraServiceName, param);

        mPipeFD = qemu_pipe_open(connection_str);
        delete[] connection_str;
    }
    if (mPipeFD < 0) {
        ALOGE("%s: Unable to connect to the camera service '%s': %s",
             __FUNCTION__, param ? param : "Factory", strerror(errno));
        return errno ? errno : EINVAL;
    }

    return NO_ERROR;
}

void QemuClient::disconnectClient()
{
    ALOGV("%s", __FUNCTION__);

    if (mPipeFD >= 0) {
        close(mPipeFD);
        mPipeFD = -1;
    }
}

status_t QemuClient::sendMessage(const void* data, size_t data_size)
{
    if (mPipeFD < 0) {
        ALOGE("%s: Qemu client is not connected", __FUNCTION__);
        return EINVAL;
    }

    /* Note that we don't use here qemud_client_send, since with qemu pipes we
     * don't need to provide payload size prior to payload when we're writing to
     * the pipe. So, we can use simple write, and qemu pipe will take care of the
     * rest, calling the receiving end with the number of bytes transferred. */
    const size_t written = qemud_fd_write(mPipeFD, data, data_size);
    if (written == data_size) {
        return NO_ERROR;
    } else {
        ALOGE("%s: Error sending data via qemu pipe: '%s'",
             __FUNCTION__, strerror(errno));
        return errno ? errno : EIO;
    }
}

status_t QemuClient::receiveMessage(void** data, size_t* data_size)
{
    *data = NULL;
    *data_size = 0;

    if (mPipeFD < 0) {
        ALOGE("%s: Qemu client is not connected", __FUNCTION__);
        return EINVAL;
    }

    /* The way the service replies to a query, it sends payload size first, and
     * then it sends the payload itself. Note that payload size is sent as a
     * string, containing 8 characters representing a hexadecimal payload size
     * value. Note also, that the string doesn't contain zero-terminator. */
    size_t payload_size;
    char payload_size_str[9];
    int rd_res = qemud_fd_read(mPipeFD, payload_size_str, 8);
    if (rd_res != 8) {
        ALOGE("%s: Unable to obtain payload size: %s",
             __FUNCTION__, strerror(errno));
        return errno ? errno : EIO;
    }

    /* Convert payload size. */
    errno = 0;
    payload_size_str[8] = '\0';
    payload_size = strtol(payload_size_str, NULL, 16);
    if (errno) {
        ALOGE("%s: Invalid payload size '%s'", __FUNCTION__, payload_size_str);
        return EIO;
    }

    /* Allocate payload data buffer, and read the payload there. */
    *data = malloc(payload_size);
    if (*data == NULL) {
        ALOGE("%s: Unable to allocate %zu bytes payload buffer",
             __FUNCTION__, payload_size);
        return ENOMEM;
    }
    rd_res = qemud_fd_read(mPipeFD, *data, payload_size);
    if (static_cast<size_t>(rd_res) == payload_size) {
        *data_size = payload_size;
        return NO_ERROR;
    } else {
        ALOGE("%s: Read size %d doesnt match expected payload size %zu: %s",
             __FUNCTION__, rd_res, payload_size, strerror(errno));
        free(*data);
        *data = NULL;
        return errno ? errno : EIO;
    }
}

status_t QemuClient::doQuery(QemuQuery* query)
{
    /* Make sure that query has been successfuly constructed. */
    if (query->mQueryDeliveryStatus != NO_ERROR) {
        ALOGE("%s: Query is invalid", __FUNCTION__);
        return query->mQueryDeliveryStatus;
    }

    LOGQ("Send query '%s'", query->mQuery);

    /* Send the query. */
    status_t res = sendMessage(query->mQuery, strlen(query->mQuery) + 1);
    if (res == NO_ERROR) {
        /* Read the response. */
        res = receiveMessage(reinterpret_cast<void**>(&query->mReplyBuffer),
                      &query->mReplySize);
        if (res == NO_ERROR) {
            LOGQ("Response to query '%s': Status = '%.2s', %d bytes in response",
                 query->mQuery, query->mReplyBuffer, query->mReplySize);
        } else {
            ALOGE("%s Response to query '%s' has failed: %s",
                 __FUNCTION__, query->mQuery, strerror(res));
        }
    } else {
        ALOGE("%s: Send query '%s' failed: %s",
             __FUNCTION__, query->mQuery, strerror(res));
    }

    /* Complete the query, and return its completion handling status. */
    const status_t res1 = query->completeQuery(res);
    ALOGE_IF(res1 != NO_ERROR && res1 != res,
            "%s: Error %d in query '%s' completion",
            __FUNCTION__, res1, query->mQuery);
    return res1;
}

/****************************************************************************
 * Qemu client for the 'factory' service.
 ***************************************************************************/

/*
 * Factory service queries.
 */

/* Queries list of cameras connected to the host. */
const char FactoryQemuClient::mQueryList[] = "list";

FactoryQemuClient::FactoryQemuClient()
    : QemuClient()
{
}

FactoryQemuClient::~FactoryQemuClient()
{
}

status_t FactoryQemuClient::listCameras(char** list)
{
    ALOGV("%s", __FUNCTION__);

    QemuQuery query(mQueryList);
    if (doQuery(&query) || !query.isQuerySucceeded()) {
        ALOGE("%s: List cameras query failed: %s", __FUNCTION__,
             query.mReplyData ? query.mReplyData : "No error message");
        return query.getCompletionStatus();
    }

    /* Make sure there is a list returned. */
    if (query.mReplyDataSize == 0) {
        ALOGE("%s: No camera list is returned.", __FUNCTION__);
        return EINVAL;
    }

    /* Copy the list over. */
    *list = (char*)malloc(query.mReplyDataSize);
    if (*list != NULL) {
        memcpy(*list, query.mReplyData, query.mReplyDataSize);
        ALOGD("Emulated camera list: %s", *list);
        return NO_ERROR;
    } else {
        ALOGE("%s: Unable to allocate %zu bytes",
             __FUNCTION__, query.mReplyDataSize);
        return ENOMEM;
    }
}

/****************************************************************************
 * Qemu client for an 'emulated camera' service.
 ***************************************************************************/

/*
 * Emulated camera queries
 */

/* Connect to the camera device. */
const char CameraQemuClient::mQueryConnect[]    = "connect";
/* Disconect from the camera device. */
const char CameraQemuClient::mQueryDisconnect[] = "disconnect";
/* Start capturing video from the camera device. */
const char CameraQemuClient::mQueryStart[]      = "start";
/* Stop capturing video from the camera device. */
const char CameraQemuClient::mQueryStop[]       = "stop";
/* Get next video frame from the camera device. */
const char CameraQemuClient::mQueryFrame[]      = "frame";

CameraQemuClient::CameraQemuClient()
    : QemuClient()
{
}

CameraQemuClient::~CameraQemuClient()
{

}

status_t CameraQemuClient::queryConnect()
{
    ALOGV("%s", __FUNCTION__);

    QemuQuery query(mQueryConnect);
    doQuery(&query);
    const status_t res = query.getCompletionStatus();
    ALOGE_IF(res != NO_ERROR, "%s: Query failed: %s",
            __FUNCTION__, query.mReplyData ? query.mReplyData :
                                             "No error message");
    return res;
}

status_t CameraQemuClient::queryDisconnect()
{
    ALOGV("%s", __FUNCTION__);

    QemuQuery query(mQueryDisconnect);
    doQuery(&query);
    const status_t res = query.getCompletionStatus();
    ALOGE_IF(res != NO_ERROR, "%s: Query failed: %s",
            __FUNCTION__, query.mReplyData ? query.mReplyData :
                                             "No error message");
    return res;
}

status_t CameraQemuClient::queryStart(uint32_t pixel_format,
                                      int width,
                                      int height)
{
    ALOGV("%s", __FUNCTION__);

    char query_str[256];
    snprintf(query_str, sizeof(query_str), "%s dim=%dx%d pix=%d",
             mQueryStart, width, height, pixel_format);
    QemuQuery query(query_str);
    doQuery(&query);
    const status_t res = query.getCompletionStatus();
    ALOGE_IF(res != NO_ERROR, "%s: Query failed: %s",
            __FUNCTION__, query.mReplyData ? query.mReplyData :
                                             "No error message");
    return res;
}

status_t CameraQemuClient::queryStop()
{
    ALOGV("%s", __FUNCTION__);

    QemuQuery query(mQueryStop);
    doQuery(&query);
    const status_t res = query.getCompletionStatus();
    ALOGE_IF(res != NO_ERROR, "%s: Query failed: %s",
            __FUNCTION__, query.mReplyData ? query.mReplyData :
                                             "No error message");
    return res;
}

status_t CameraQemuClient::queryFrame(void* vframe,
                                      void* pframe,
                                      size_t vframe_size,
                                      size_t pframe_size,
                                      float r_scale,
                                      float g_scale,
                                      float b_scale,
                                      float exposure_comp)
{
    ALOGV("%s", __FUNCTION__);

    char query_str[256];
    snprintf(query_str, sizeof(query_str), "%s video=%zu preview=%zu whiteb=%g,%g,%g expcomp=%g",
             mQueryFrame, (vframe && vframe_size) ? vframe_size : 0,
             (pframe && pframe_size) ? pframe_size : 0, r_scale, g_scale, b_scale,
             exposure_comp);
    QemuQuery query(query_str);
    doQuery(&query);
    const status_t res = query.getCompletionStatus();
    if( res != NO_ERROR) {
        ALOGE("%s: Query failed: %s",
             __FUNCTION__, query.mReplyData ? query.mReplyData :
                                              "No error message");
        return res;
    }

    /* Copy requested frames. */
    size_t cur_offset = 0;
    const uint8_t* frame = reinterpret_cast<const uint8_t*>(query.mReplyData);
    /* Video frame is always first. */
    if (vframe != NULL && vframe_size != 0) {
        /* Make sure that video frame is in. */
        if ((query.mReplyDataSize - cur_offset) >= vframe_size) {
            memcpy(vframe, frame, vframe_size);
            cur_offset += vframe_size;
        } else {
            ALOGE("%s: Reply %zu bytes is to small to contain %zu bytes video frame",
                 __FUNCTION__, query.mReplyDataSize - cur_offset, vframe_size);
            return EINVAL;
        }
    }
    if (pframe != NULL && pframe_size != 0) {
        /* Make sure that preview frame is in. */
        if ((query.mReplyDataSize - cur_offset) >= pframe_size) {
            memcpy(pframe, frame + cur_offset, pframe_size);
            cur_offset += pframe_size;
        } else {
            ALOGE("%s: Reply %zu bytes is to small to contain %zu bytes preview frame",
                 __FUNCTION__, query.mReplyDataSize - cur_offset, pframe_size);
            return EINVAL;
        }
    }

    return NO_ERROR;
}

}; /* namespace android */
