/*
 * Copyright (C) 2015 The Android Open Source Project
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

/**
 * This is a very basic implementation of fingerprint to allow testing on the emulator. It
 * is *not* meant to be the final implementation on real devices.  For example,  it does *not*
 * implement all of the required features, such as secure template storage and recognition
 * inside a Trusted Execution Environment (TEE). However, this file is a reasonable starting
 * point as developers add fingerprint support to their platform.  See inline comments and
 * recommendations for details.
 *
 * Please see the Android Compatibility Definition Document (CDD) for a full list of requirements
 * and suggestions.
 */
#define LOG_TAG "FingerprintHal"

#include <errno.h>
#include <endian.h>
#include <inttypes.h>
#include <malloc.h>
#include <string.h>
#include <cutils/log.h>
#include <hardware/hardware.h>
#include <hardware/fingerprint.h>
#include <hardware/qemud.h>

#include <poll.h>

#define FINGERPRINT_LISTEN_SERVICE_NAME "fingerprintlisten"
#define FINGERPRINT_FILENAME "emufp.bin"
#define AUTHENTICATOR_ID_FILENAME "emuauthid.bin"
#define MAX_COMM_CHARS 128
#define MAX_COMM_ERRORS 8
// Typical devices will allow up to 5 fingerprints per user to maintain performance of
// t < 500ms for recognition.  This is the total number of fingerprints we'll store.
#define MAX_NUM_FINGERS 20
#define MAX_FID_VALUE 0x7FFFFFFF  // Arbitrary limit

/**
 * Most devices will have an internal state machine resembling this. There are 3 basic states, as
 * shown below. When device is not authenticating or enrolling, it is expected to be in
 * the idle state.
 *
 * Note that this is completely independent of device wake state.  If the hardware device was in
 * the "scan" state when the device drops into power collapse, it should resume scanning when power
 * is restored.  This is to facilitate rapid touch-to-unlock from keyguard.
 */
typedef enum worker_state_t {
    STATE_IDLE = 0,
    STATE_ENROLL,
    STATE_SCAN,
    STATE_EXIT
} worker_state_t;

typedef struct worker_thread_t {
    pthread_t thread;
    worker_state_t state;
    uint64_t secureid[MAX_NUM_FINGERS];
    uint64_t fingerid[MAX_NUM_FINGERS];
    char fp_filename[PATH_MAX];
    char authid_filename[PATH_MAX];
} worker_thread_t;

typedef struct qemu_fingerprint_device_t {
    fingerprint_device_t device;  // "inheritance"
    worker_thread_t listener;
    uint64_t op_id;
    uint64_t challenge;
    uint64_t user_id;
    uint64_t group_id;
    uint64_t secure_user_id;
    uint64_t authenticator_id;
    int qchanfd;
    pthread_mutex_t lock;
} qemu_fingerprint_device_t;

/******************************************************************************/

static FILE* openForWrite(const char* filename);

static void saveFingerprint(worker_thread_t* listener, int idx) {
    ALOGD("----------------> %s -----------------> idx %d", __FUNCTION__, idx);

    // Save fingerprints to file
    FILE* fp = openForWrite(listener->fp_filename);
    if (fp == NULL) {
        ALOGE("Could not open fingerprints storage at %s; "
              "fingerprints won't be saved",
              listener->fp_filename);
        perror("Failed to open file");
        return;
    }

    ALOGD("Write fingerprint[%d] (0x%" PRIx64 ",0x%" PRIx64 ")", idx,
          listener->secureid[idx], listener->fingerid[idx]);

    if (fseek(fp, (idx) * sizeof(uint64_t), SEEK_SET) < 0) {
        ALOGE("Failed while seeking for fingerprint[%d] in emulator storage",
              idx);
        fclose(fp);
        return;
    }
    int ns = fwrite(&listener->secureid[idx], sizeof(uint64_t), 1, fp);

    if (fseek(fp, (MAX_NUM_FINGERS + idx) * sizeof(uint64_t), SEEK_SET) < 0) {
        ALOGE("Failed while seeking for fingerprint[%d] in emulator storage",
              idx);
        fclose(fp);
        return;
    }
    int nf = fwrite(&listener->fingerid[idx], sizeof(uint64_t), 1, fp);
    if (ns != 1 || ns !=1)
        ALOGW("Corrupt emulator fingerprints storage; could not save "
              "fingerprints");

    fclose(fp);

    return;
}

static FILE* openForWrite(const char* filename) {

    if (!filename) return NULL;

    FILE* fp = fopen(filename, "r+");  // write but don't truncate
    if (fp == NULL) {
        fp = fopen(filename, "w");
        if (fp) {
            uint64_t zero = 0;
            int i = 0;
            for (i = 0; i < 2*MAX_NUM_FINGERS; ++i) {
                fwrite(&zero, sizeof(uint64_t), 1, fp);
            }

            //the last one is for authenticator id
            fwrite(&zero, sizeof(uint64_t), 1, fp);
        }
    }
    return fp;
}

static void saveAuthenticatorId(const char* filename, uint64_t authenid) {
    ALOGD("----------------> %s ----------------->", __FUNCTION__);
    FILE* fp = openForWrite(filename);
    if (!fp) {
        ALOGE("Failed to open emulator storage file to save authenticator id");
        return;
    }

    rewind(fp);

    int na = fwrite(&authenid, sizeof(authenid), 1, fp);
    if (na != 1) {
        ALOGE("Failed while writing authenticator id in emulator storage");
    }

    ALOGD("Save authenticator id (0x%" PRIx64 ")", authenid);

    fclose(fp);
}

static void loadAuthenticatorId(const char* authid_filename, uint64_t* pauthenid) {
    ALOGD("----------------> %s ----------------->", __FUNCTION__);
    FILE* fp = fopen(authid_filename, "r");
    if (fp == NULL) {
        ALOGE("Could not load authenticator id from storage at %s; "
              "it has not yet been created.",
              authid_filename);
        perror("Failed to open/create file");
        return;
    }

    rewind(fp);

    int na = fread(pauthenid, sizeof(*pauthenid), 1, fp);
    if (na != 1)
        ALOGW("Corrupt emulator authenticator id storage (read %d)", na);

    ALOGD("Read authenticator id (0x%" PRIx64 ")", *pauthenid);

    fclose(fp);

    return;
}

static void loadFingerprints(worker_thread_t* listener) {
    ALOGD("----------------> %s ----------------->", __FUNCTION__);
    FILE* fp = fopen(listener->fp_filename, "r");
    if (fp == NULL) {
        ALOGE("Could not load fingerprints from storage at %s; "
              "it has not yet been created.",
              listener->fp_filename);
        perror("Failed to open/create file");
        return;
    }

    int ns = fread(listener->secureid, MAX_NUM_FINGERS * sizeof(uint64_t), 1,
                   fp);
    int nf = fread(listener->fingerid, MAX_NUM_FINGERS * sizeof(uint64_t), 1,
                   fp);
    if (ns != 1 || nf != 1)
        ALOGW("Corrupt emulator fingerprints storage (read %d+%db)", ns, nf);

    int i = 0;
    for (i = 0; i < MAX_NUM_FINGERS; i++)
        ALOGD("Read fingerprint %d (0x%" PRIx64 ",0x%" PRIx64 ")", i,
              listener->secureid[i], listener->fingerid[i]);

    fclose(fp);

    return;
}

/******************************************************************************/

static uint64_t get_64bit_rand() {
    // This should use a cryptographically-secure random number generator like arc4random().
    // It should be generated inside of the TEE where possible. Here we just use something
    // very simple.
    ALOGD("----------------> %s ----------------->", __FUNCTION__);
    uint64_t r = (((uint64_t)rand()) << 32) | ((uint64_t)rand());
    return r != 0 ? r : 1;
}

static uint64_t fingerprint_get_auth_id(struct fingerprint_device* device) {
    // This should return the authentication_id generated when the fingerprint template database
    // was created.  Though this isn't expected to be secret, it is reasonable to expect it to be
    // cryptographically generated to avoid replay attacks.
    qemu_fingerprint_device_t* qdev = (qemu_fingerprint_device_t*)device;
    ALOGD("----------------> %s ----------------->", __FUNCTION__);
    uint64_t authenticator_id = 0;
    pthread_mutex_lock(&qdev->lock);
    authenticator_id = qdev->authenticator_id;
    pthread_mutex_unlock(&qdev->lock);

    ALOGD("----------------> %s auth id %" PRIx64 "----------------->", __FUNCTION__, authenticator_id);
    return authenticator_id;
}

static int fingerprint_set_active_group(struct fingerprint_device *device, uint32_t gid,
        const char *path) {
    ALOGD("----------------> %s -----------------> path %s", __FUNCTION__, path);
    qemu_fingerprint_device_t* qdev = (qemu_fingerprint_device_t*)device;
    pthread_mutex_lock(&qdev->lock);
    qdev->group_id = gid;
    snprintf(qdev->listener.fp_filename, sizeof(qdev->listener.fp_filename),
            "%s/%s", path, FINGERPRINT_FILENAME);
    snprintf(qdev->listener.authid_filename, sizeof(qdev->listener.authid_filename),
            "%s/%s", path, AUTHENTICATOR_ID_FILENAME);
    uint64_t authenticator_id = 0;
    loadFingerprints(&qdev->listener);
    loadAuthenticatorId(qdev->listener.authid_filename, &authenticator_id);
    if (authenticator_id == 0) {
        // firs time, create an authenticator id
        authenticator_id = get_64bit_rand();
        // save it to disk
        saveAuthenticatorId(qdev->listener.authid_filename, authenticator_id);
    }

    qdev->authenticator_id = authenticator_id;
    pthread_mutex_unlock(&qdev->lock);

    return 0;
}

/**
 * If fingerprints are enrolled, then this function is expected to put the sensor into a
 * "scanning" state where it's actively scanning and recognizing fingerprint features.
 * Actual authentication must happen in TEE and should be monitored in a separate thread
 * since this function is expected to return immediately.
 */
static int fingerprint_authenticate(struct fingerprint_device *device,
    uint64_t operation_id, __unused uint32_t gid)
{
    qemu_fingerprint_device_t* qdev = (qemu_fingerprint_device_t*)device;

    pthread_mutex_lock(&qdev->lock);
    qdev->op_id = operation_id;
    qdev->listener.state = STATE_SCAN;
    pthread_mutex_unlock(&qdev->lock);

    return 0;
}

/**
 * This is expected to put the sensor into an "enroll" state where it's actively scanning and
 * working towards a finished fingerprint database entry. Authentication must happen in
 * a separate thread since this function is expected to return immediately.
 *
 * Note: This method should always generate a new random authenticator_id.
 *
 * Note: As with fingerprint_authenticate(), this would run in TEE on a real device.
 */
static int fingerprint_enroll(struct fingerprint_device *device,
        const hw_auth_token_t *hat,
        uint32_t __unused gid,
        uint32_t __unused timeout_sec) {
    ALOGD("fingerprint_enroll");
    qemu_fingerprint_device_t* dev = (qemu_fingerprint_device_t*)device;
    if (!hat) {
        ALOGW("%s: null auth token", __func__);
        return -EPROTONOSUPPORT;
    }
    if (hat->challenge == dev->challenge) {
        // The secure_user_id retrieved from the auth token should be stored
        // with the enrolled fingerprint template and returned in the auth result
        // for a successful authentication with that finger.
        dev->secure_user_id = hat->user_id;
    } else {
        ALOGW("%s: invalid auth token", __func__);
    }

    if (hat->version != HW_AUTH_TOKEN_VERSION) {
        return -EPROTONOSUPPORT;
    }
    if (hat->challenge != dev->challenge && !(hat->authenticator_type & HW_AUTH_FINGERPRINT)) {
        return -EPERM;
    }

    dev->user_id = hat->user_id;

    pthread_mutex_lock(&dev->lock);
    dev->listener.state = STATE_ENROLL;
    pthread_mutex_unlock(&dev->lock);

    // fingerprint id, authenticator id, and secure_user_id
    // will be stored by worked thread

    return 0;

}

/**
 * The pre-enrollment step is simply to get an authentication token that can be wrapped and
 * verified at a later step.  The primary purpose is to return a token that protects against
 * spoofing and replay attacks. It is passed to password authentication where it is wrapped and
 * propagated to the enroll step.
 */
static uint64_t fingerprint_pre_enroll(struct fingerprint_device *device) {
    ALOGD("----------------> %s ----------------->", __FUNCTION__);
    uint64_t challenge = 0;
    qemu_fingerprint_device_t* qdev = (qemu_fingerprint_device_t*)device;

    // The challenge will typically be a cryptographically-secure key
    // coming from the TEE so it can be verified at a later step. For now we just generate a
    // random value.
    challenge = get_64bit_rand();

    pthread_mutex_lock(&qdev->lock);
    qdev->challenge = challenge;
    pthread_mutex_unlock(&qdev->lock);

    return challenge;
}

static int fingerprint_post_enroll(struct fingerprint_device* device) {
    ALOGD("----------------> %s ----------------->", __FUNCTION__);
    qemu_fingerprint_device_t* qdev = (qemu_fingerprint_device_t*)device;

    pthread_mutex_lock(&qdev->lock);
    qdev->challenge = 0;
    pthread_mutex_unlock(&qdev->lock);

    return 0;
}

/**
 * Cancel is called by the framework to cancel an outstanding event.  This should *not* be called
 * by the driver since it will cause the framework to stop listening for fingerprints.
 */
static int fingerprint_cancel(struct fingerprint_device *device) {
    ALOGD("----------------> %s ----------------->", __FUNCTION__);
    qemu_fingerprint_device_t* qdev = (qemu_fingerprint_device_t*)device;

    pthread_mutex_lock(&qdev->lock);
    qdev->listener.state = STATE_IDLE;
    pthread_mutex_unlock(&qdev->lock);

    fingerprint_msg_t msg = {0, {0}};
    msg.type = FINGERPRINT_ERROR;
    msg.data.error = FINGERPRINT_ERROR_CANCELED;
    qdev->device.notify(&msg);

    return 0;
}

static int fingerprint_enumerate(struct fingerprint_device *device) {
    ALOGD("----------------> %s ----------------->", __FUNCTION__);
    if (device == NULL) {
        ALOGE("Cannot enumerate saved fingerprints with uninitialized params");
        return -1;
    }

    qemu_fingerprint_device_t* qdev = (qemu_fingerprint_device_t*)device;
    int template_count = 0;
    for (int i = 0; i < MAX_NUM_FINGERS; i++) {
        if (qdev->listener.secureid[i] != 0 ||
            qdev->listener.fingerid[i] != 0) {
            ALOGD("ENUM: Fingerprint [%d] = 0x%" PRIx64 ",%" PRIx64, i,
                  qdev->listener.secureid[i], qdev->listener.fingerid[i]);
            template_count++;
        }
    }
    fingerprint_msg_t message = {0, {0}};
    message.type = FINGERPRINT_TEMPLATE_ENUMERATING;
    message.data.enumerated.finger.gid = qdev->group_id;
    for (int i = 0; i < MAX_NUM_FINGERS; i++) {
        if (qdev->listener.secureid[i] != 0 ||
            qdev->listener.fingerid[i] != 0) {
            template_count--;
            message.data.enumerated.remaining_templates = template_count;
            message.data.enumerated.finger.fid = qdev->listener.fingerid[i];
            qdev->device.notify(&message);
        }
    }

    return 0;
}

static int fingerprint_remove(struct fingerprint_device *device,
        uint32_t __unused gid, uint32_t fid) {
    int idx = 0;
    fingerprint_msg_t msg = {0, {0}};
    ALOGD("----------------> %s -----------------> fid %d", __FUNCTION__, fid);
    if (device == NULL) {
        ALOGE("Can't remove fingerprint (gid=%d, fid=%d); "
              "device not initialized properly",
              gid, fid);
        return -1;
    }

    qemu_fingerprint_device_t* qdev = (qemu_fingerprint_device_t*)device;

    if (fid == 0) {
        // Delete all fingerprints
        // I'll do this one at a time, so I am not
        // holding the mutext during the notification
        bool listIsEmpty;
        do {
            pthread_mutex_lock(&qdev->lock);
            listIsEmpty = true;  // Haven't seen a valid entry yet
            for (idx = 0; idx < MAX_NUM_FINGERS; idx++) {
                uint32_t theFid = qdev->listener.fingerid[idx];
                if (theFid != 0) {
                    // Delete this entry
                    qdev->listener.secureid[idx] = 0;
                    qdev->listener.fingerid[idx] = 0;
                    saveFingerprint(&qdev->listener, idx);

                    // Send a notification that we deleted this one
                    pthread_mutex_unlock(&qdev->lock);
                    msg.type = FINGERPRINT_TEMPLATE_REMOVED;
                    msg.data.removed.finger.fid = theFid;
                    device->notify(&msg);

                    // Because we released the mutex, the list
                    // may have changed. Restart the 'for' loop
                    // after reacquiring the mutex.
                    listIsEmpty = false;
                    break;
                }
            }  // end for (idx < MAX_NUM_FINGERS)
        } while (!listIsEmpty);
        msg.type = FINGERPRINT_TEMPLATE_REMOVED;
        msg.data.removed.finger.fid = 0;
        device->notify(&msg);
        qdev->listener.state = STATE_IDLE;
        pthread_mutex_unlock(&qdev->lock);
    } else {
        // Delete one fingerprint
        // Look for this finger ID in our table.
        pthread_mutex_lock(&qdev->lock);
        for (idx = 0; idx < MAX_NUM_FINGERS; idx++) {
            if (qdev->listener.fingerid[idx] == fid &&
                qdev->listener.secureid[idx] != 0) {
                // Found it!
                break;
            }
        }
        if (idx >= MAX_NUM_FINGERS) {
            qdev->listener.state = STATE_IDLE;
            pthread_mutex_unlock(&qdev->lock);
            ALOGE("Fingerprint ID %d not found", fid);
            return FINGERPRINT_ERROR;
        }

        qdev->listener.secureid[idx] = 0;
        qdev->listener.fingerid[idx] = 0;
        saveFingerprint(&qdev->listener, idx);

        qdev->listener.state = STATE_IDLE;
        pthread_mutex_unlock(&qdev->lock);

        msg.type = FINGERPRINT_TEMPLATE_REMOVED;
        msg.data.removed.finger.fid = fid;
        device->notify(&msg);
    }

    return 0;
}

static int set_notify_callback(struct fingerprint_device *device,
                               fingerprint_notify_t notify) {
    ALOGD("----------------> %s ----------------->", __FUNCTION__);
    if (device == NULL || notify == NULL) {
        ALOGE("Failed to set notify callback @ %p for fingerprint device %p",
              device, notify);
        return -1;
    }

    qemu_fingerprint_device_t* qdev = (qemu_fingerprint_device_t*)device;
    pthread_mutex_lock(&qdev->lock);
    qdev->listener.state = STATE_IDLE;
    device->notify = notify;
    pthread_mutex_unlock(&qdev->lock);
    ALOGD("fingerprint callback notification set");

    return 0;
}

static bool is_valid_fid(qemu_fingerprint_device_t* qdev, uint64_t fid) {
    int idx = 0;
    if (0 == fid) { return false; }
    for (idx = 0; idx < MAX_NUM_FINGERS; idx++) {
        if (qdev->listener.fingerid[idx] == fid) {
            return true;
        }
    }
    return false;
}

static void send_scan_notice(qemu_fingerprint_device_t* qdev, int fid) {
    ALOGD("----------------> %s ----------------->", __FUNCTION__);

    // acquired message
    fingerprint_msg_t acqu_msg = {0, {0}};
    acqu_msg.type = FINGERPRINT_ACQUIRED;
    acqu_msg.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_GOOD;

    // authenticated message
    fingerprint_msg_t auth_msg = {0, {0}};
    auth_msg.type = FINGERPRINT_AUTHENTICATED;
    auth_msg.data.authenticated.finger.fid = is_valid_fid(qdev, fid) ? fid : 0;
    auth_msg.data.authenticated.finger.gid = 0;  // unused
    auth_msg.data.authenticated.hat.version = HW_AUTH_TOKEN_VERSION;
    auth_msg.data.authenticated.hat.authenticator_type =
            htobe32(HW_AUTH_FINGERPRINT);
    auth_msg.data.authenticated.hat.challenge = qdev->op_id;
    auth_msg.data.authenticated.hat.authenticator_id = qdev->authenticator_id;
    auth_msg.data.authenticated.hat.user_id = qdev->secure_user_id;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    auth_msg.data.authenticated.hat.timestamp =
            htobe64((uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000);

    //  pthread_mutex_lock(&qdev->lock);
    qdev->device.notify(&acqu_msg);
    qdev->device.notify(&auth_msg);
    //  pthread_mutex_unlock(&qdev->lock);

    return;
}

static void send_enroll_notice(qemu_fingerprint_device_t* qdev, int fid) {
    ALOGD("----------------> %s -----------------> fid %d", __FUNCTION__, fid);

    if (fid == 0) {
        ALOGD("Fingerprint ID is zero (invalid)");
        return;
    }
    if (qdev->secure_user_id == 0) {
        ALOGD("Secure user ID is zero (invalid)");
        return;
    }

    // Find an available entry in the table
    pthread_mutex_lock(&qdev->lock);
    int idx = 0;
    for (idx = 0; idx < MAX_NUM_FINGERS; idx++) {
        if (qdev->listener.secureid[idx] == 0 ||
            qdev->listener.fingerid[idx] == 0) {
            // This entry is available
            break;
        }
    }
    if (idx >= MAX_NUM_FINGERS) {
        qdev->listener.state = STATE_SCAN;
        pthread_mutex_unlock(&qdev->lock);
        ALOGD("Fingerprint ID table is full");
        return;
    }

    qdev->listener.secureid[idx] = qdev->secure_user_id;
    qdev->listener.fingerid[idx] = fid;
    saveFingerprint(&qdev->listener, idx);

    qdev->listener.state = STATE_SCAN;
    pthread_mutex_unlock(&qdev->lock);

    // LOCKED notification?
    fingerprint_msg_t msg = {0, {0}};
    msg.type = FINGERPRINT_TEMPLATE_ENROLLING;
    msg.data.enroll.finger.fid = fid;
    msg.data.enroll.samples_remaining = 0;
    qdev->device.notify(&msg);

    return;
}

static worker_state_t getListenerState(qemu_fingerprint_device_t* dev) {
    ALOGV("----------------> %s ----------------->", __FUNCTION__);
    worker_state_t state = STATE_IDLE;

    pthread_mutex_lock(&dev->lock);
    state = dev->listener.state;
    pthread_mutex_unlock(&dev->lock);

    return state;
}

/**
 * This a very simple event loop for the fingerprint sensor. For a given state (enroll, scan),
 * this would receive events from the sensor and forward them to fingerprintd using the
 * notify() method.
 *
 * In this simple example, we open a qemu channel (a pipe) where the developer can inject events to
 * exercise the API and test application code.
 *
 * The scanner should remain in the scanning state until either an error occurs or the operation
 * completes.
 *
 * Recoverable errors such as EINTR should be handled locally;  they should not
 * be propagated unless there's something the user can do about it (e.g. "clean sensor"). Such
 * messages should go through the onAcquired() interface.
 *
 * If an unrecoverable error occurs, an acquired message (e.g. ACQUIRED_PARTIAL) should be sent,
 * followed by an error message (e.g. FINGERPRINT_ERROR_UNABLE_TO_PROCESS).
 *
 * Note that this event loop would typically run in TEE since it must interact with the sensor
 * hardware and handle raw fingerprint data and encrypted templates.  It is expected that
 * this code monitors the TEE for resulting events, such as enrollment and authentication status.
 * Here we just have a very simple event loop that monitors a qemu channel for pseudo events.
 */
static void* listenerFunction(void* data) {
    ALOGD("----------------> %s ----------------->", __FUNCTION__);
    qemu_fingerprint_device_t* qdev = (qemu_fingerprint_device_t*)data;

    pthread_mutex_lock(&qdev->lock);
    qdev->qchanfd = qemud_channel_open(FINGERPRINT_LISTEN_SERVICE_NAME);
    if (qdev->qchanfd < 0) {
        ALOGE("listener cannot open fingerprint listener service exit");
        pthread_mutex_unlock(&qdev->lock);
        return NULL;
    }
    qdev->listener.state = STATE_IDLE;
    pthread_mutex_unlock(&qdev->lock);

    const char* cmd = "listen";
    if (qemud_channel_send(qdev->qchanfd, cmd, strlen(cmd)) < 0) {
        ALOGE("cannot write fingerprint 'listen' to host");
        goto done_quiet;
    }

    int comm_errors = 0;
    struct pollfd pfd = {
        .fd = qdev->qchanfd,
        .events = POLLIN,
    };
    while (1) {
        int size = 0;
        int fid = 0;
        char buffer[MAX_COMM_CHARS] = {0};
        bool disconnected = false;
        while (1) {
            if (getListenerState(qdev) == STATE_EXIT) {
                ALOGD("Received request to exit listener thread");
                goto done;
            }

            // Reset revents before poll() (just to be safe)
            pfd.revents = 0;

            // Poll qemud channel for 5 seconds
            // TODO: Eliminate the timeout so that polling can be interrupted
            // instantly. One possible solution is to follow the example of
            // android::Looper ($AOSP/system/core/include/utils/Looper.h and
            // $AOSP/system/core/libutils/Looper.cpp), which makes use of an
            // additional file descriptor ("wake event fd").
            int nfds = poll(&pfd, 1, 5000);
            if (nfds < 0) {
                ALOGE("Could not poll qemud channel: %s", strerror(errno));
                goto done;
            }

            if (!nfds) {
                // poll() timed out - try again
                continue;
            }

            // assert(nfds == 1)
            if (pfd.revents & POLLIN) {
                // Input data being available doesn't rule out a disconnection
                disconnected = pfd.revents & (POLLERR | POLLHUP);
                break;  // Exit inner while loop
            } else {
                // Some event(s) other than "input data available" occurred,
                // i.e. POLLERR or POLLHUP, indicating a disconnection
                ALOGW("Lost connection to qemud channel");
                goto done;
            }
        }

        // Shouldn't block since we were just notified of a POLLIN event
        if ((size = qemud_channel_recv(qdev->qchanfd, buffer,
                                       sizeof(buffer) - 1)) > 0) {
            buffer[size] = '\0';
            if (sscanf(buffer, "on:%d", &fid) == 1) {
                if (fid > 0 && fid <= MAX_FID_VALUE) {
                    switch (qdev->listener.state) {
                        case STATE_ENROLL:
                            send_enroll_notice(qdev, fid);
                            break;
                        case STATE_SCAN:
                            send_scan_notice(qdev, fid);
                            break;
                        default:
                            ALOGE("fingerprint event listener at unexpected "
                                  "state 0%x",
                                  qdev->listener.state);
                    }
                } else {
                    ALOGE("fingerprintid %d not in valid range [%d, %d] and "
                          "will be "
                          "ignored",
                          fid, 1, MAX_FID_VALUE);
                    continue;
                }
            } else if (strncmp("off", buffer, 3) == 0) {
                // TODO: Nothing to do here ? Looks valid
                ALOGD("fingerprint ID %d off", fid);
            } else {
                ALOGE("Invalid command '%s' to fingerprint listener", buffer);
            }

            if (disconnected) {
                ALOGW("Connection to qemud channel has been lost");
                break;
            }
        } else {
            ALOGE("fingerprint listener receive failure");
            if (comm_errors > MAX_COMM_ERRORS)
                break;
        }
    }

done:
    ALOGD("Listener exit with %d receive errors", comm_errors);
done_quiet:
    close(qdev->qchanfd);
    return NULL;
}

static int fingerprint_close(hw_device_t* device) {
    ALOGD("----------------> %s ----------------->", __FUNCTION__);
    if (device == NULL) {
        ALOGE("fingerprint hw device is NULL");
        return -1;
    }

    qemu_fingerprint_device_t* qdev = (qemu_fingerprint_device_t*)device;
    pthread_mutex_lock(&qdev->lock);
    // Ask listener thread to exit
    qdev->listener.state = STATE_EXIT;
    pthread_mutex_unlock(&qdev->lock);

    pthread_join(qdev->listener.thread, NULL);
    pthread_mutex_destroy(&qdev->lock);
    free(qdev);

    return 0;
}

static int fingerprint_open(const hw_module_t* module, const char __unused *id,
                            hw_device_t** device)
{

    ALOGD("----------------> %s ----------------->", __FUNCTION__);
    if (device == NULL) {
        ALOGE("NULL device on open");
        return -EINVAL;
    }

    qemu_fingerprint_device_t* qdev = (qemu_fingerprint_device_t*)calloc(
            1, sizeof(qemu_fingerprint_device_t));
    if (qdev == NULL) {
        ALOGE("Insufficient memory for virtual fingerprint device");
        return -ENOMEM;
    }


    qdev->device.common.tag = HARDWARE_DEVICE_TAG;
    qdev->device.common.version = HARDWARE_MODULE_API_VERSION(2, 1);
    qdev->device.common.module = (struct hw_module_t*)module;
    qdev->device.common.close = fingerprint_close;

    qdev->device.pre_enroll = fingerprint_pre_enroll;
    qdev->device.enroll = fingerprint_enroll;
    qdev->device.post_enroll = fingerprint_post_enroll;
    qdev->device.get_authenticator_id = fingerprint_get_auth_id;
    qdev->device.set_active_group = fingerprint_set_active_group;
    qdev->device.authenticate = fingerprint_authenticate;
    qdev->device.cancel = fingerprint_cancel;
    qdev->device.enumerate = fingerprint_enumerate;
    qdev->device.remove = fingerprint_remove;
    qdev->device.set_notify = set_notify_callback;
    qdev->device.notify = NULL;

    // init and create listener thread
    pthread_mutex_init(&qdev->lock, NULL);
    if (pthread_create(&qdev->listener.thread, NULL, listenerFunction, qdev) !=
        0)
        return -1;

    // "Inheritance" / casting
    *device = &qdev->device.common;

    return 0;
}

static struct hw_module_methods_t fingerprint_module_methods = {
    .open = fingerprint_open,
};

fingerprint_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag                = HARDWARE_MODULE_TAG,
        .module_api_version = FINGERPRINT_MODULE_API_VERSION_2_1,
        .hal_api_version    = HARDWARE_HAL_API_VERSION,
        .id                 = FINGERPRINT_HARDWARE_MODULE_ID,
        .name               = "Emulator Fingerprint HAL",
        .author             = "The Android Open Source Project",
        .methods            = &fingerprint_module_methods,
    },
};
