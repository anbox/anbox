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

/**
 * This class is a simple simulation of a typical CMOS cellphone imager chip,
 * which outputs 12-bit Bayer-mosaic raw images.
 *
 * Unlike most real image sensors, this one's native color space is linear sRGB.
 *
 * The sensor is abstracted as operating as a pipeline 3 stages deep;
 * conceptually, each frame to be captured goes through these three stages. The
 * processing step for the sensor is marked off by vertical sync signals, which
 * indicate the start of readout of the oldest frame. The interval between
 * processing steps depends on the frame duration of the frame currently being
 * captured. The stages are 1) configure, 2) capture, and 3) readout. During
 * configuration, the sensor's registers for settings such as exposure time,
 * frame duration, and gain are set for the next frame to be captured. In stage
 * 2, the image data for the frame is actually captured by the sensor. Finally,
 * in stage 3, the just-captured data is read out and sent to the rest of the
 * system.
 *
 * The sensor is assumed to be rolling-shutter, so low-numbered rows of the
 * sensor are exposed earlier in time than larger-numbered rows, with the time
 * offset between each row being equal to the row readout time.
 *
 * The characteristics of this sensor don't correspond to any actual sensor,
 * but are not far off typical sensors.
 *
 * Example timing diagram, with three frames:
 *  Frame 0-1: Frame duration 50 ms, exposure time 20 ms.
 *  Frame   2: Frame duration 75 ms, exposure time 65 ms.
 * Legend:
 *   C = update sensor registers for frame
 *   v = row in reset (vertical blanking interval)
 *   E = row capturing image data
 *   R = row being read out
 *   | = vertical sync signal
 *time(ms)|   0          55        105       155            230     270
 * Frame 0|   :configure : capture : readout :              :       :
 *  Row # | ..|CCCC______|_________|_________|              :       :
 *      0 |   :\          \vvvvvEEEER         \             :       :
 *    500 |   : \          \vvvvvEEEER         \            :       :
 *   1000 |   :  \          \vvvvvEEEER         \           :       :
 *   1500 |   :   \          \vvvvvEEEER         \          :       :
 *   2000 |   :    \__________\vvvvvEEEER_________\         :       :
 * Frame 1|   :           configure  capture      readout   :       :
 *  Row # |   :          |CCCC_____|_________|______________|       :
 *      0 |   :          :\         \vvvvvEEEER              \      :
 *    500 |   :          : \         \vvvvvEEEER              \     :
 *   1000 |   :          :  \         \vvvvvEEEER              \    :
 *   1500 |   :          :   \         \vvvvvEEEER              \   :
 *   2000 |   :          :    \_________\vvvvvEEEER______________\  :
 * Frame 2|   :          :          configure     capture    readout:
 *  Row # |   :          :         |CCCC_____|______________|_______|...
 *      0 |   :          :         :\         \vEEEEEEEEEEEEER       \
 *    500 |   :          :         : \         \vEEEEEEEEEEEEER       \
 *   1000 |   :          :         :  \         \vEEEEEEEEEEEEER       \
 *   1500 |   :          :         :   \         \vEEEEEEEEEEEEER       \
 *   2000 |   :          :         :    \_________\vEEEEEEEEEEEEER_______\
 */

#ifndef HW_EMULATOR_CAMERA2_SENSOR_H
#define HW_EMULATOR_CAMERA2_SENSOR_H

#include "utils/Thread.h"
#include "utils/Mutex.h"
#include "utils/Timers.h"

#include "Scene.h"
#include "Base.h"

namespace android {

class EmulatedFakeCamera2;

class Sensor: private Thread, public virtual RefBase {
  public:

    Sensor();
    ~Sensor();

    /*
     * Power control
     */

    status_t startUp();
    status_t shutDown();

    /*
     * Access to scene
     */
    Scene &getScene();

    /*
     * Controls that can be updated every frame
     */

    void setExposureTime(uint64_t ns);
    void setFrameDuration(uint64_t ns);
    void setSensitivity(uint32_t gain);
    // Buffer must be at least stride*height*2 bytes in size
    void setDestinationBuffers(Buffers *buffers);
    // To simplify tracking sensor's current frame
    void setFrameNumber(uint32_t frameNumber);

    /*
     * Controls that cause reconfiguration delay
     */

    void setBinning(int horizontalFactor, int verticalFactor);

    /*
     * Synchronizing with sensor operation (vertical sync)
     */

    // Wait until the sensor outputs its next vertical sync signal, meaning it
    // is starting readout of its latest frame of data. Returns true if vertical
    // sync is signaled, false if the wait timed out.
    bool waitForVSync(nsecs_t reltime);

    // Wait until a new frame has been read out, and then return the time
    // capture started.  May return immediately if a new frame has been pushed
    // since the last wait for a new frame. Returns true if new frame is
    // returned, false if timed out.
    bool waitForNewFrame(nsecs_t reltime,
            nsecs_t *captureTime);

    /*
     * Interrupt event servicing from the sensor. Only triggers for sensor
     * cycles that have valid buffers to write to.
     */
    struct SensorListener {
        enum Event {
            EXPOSURE_START, // Start of exposure
        };

        virtual void onSensorEvent(uint32_t frameNumber, Event e,
                nsecs_t timestamp) = 0;
        virtual ~SensorListener();
    };

    void setSensorListener(SensorListener *listener);

    /**
     * Static sensor characteristics
     */
    static const unsigned int kResolution[2];
    static const unsigned int kActiveArray[4];

    static const nsecs_t kExposureTimeRange[2];
    static const nsecs_t kFrameDurationRange[2];
    static const nsecs_t kMinVerticalBlank;

    static const uint8_t kColorFilterArrangement;

    // Output image data characteristics
    static const uint32_t kMaxRawValue;
    static const uint32_t kBlackLevel;
    // Sensor sensitivity, approximate

    static const float kSaturationVoltage;
    static const uint32_t kSaturationElectrons;
    static const float kVoltsPerLuxSecond;
    static const float kElectronsPerLuxSecond;

    static const float kBaseGainFactor;

    static const float kReadNoiseStddevBeforeGain; // In electrons
    static const float kReadNoiseStddevAfterGain;  // In raw digital units
    static const float kReadNoiseVarBeforeGain;
    static const float kReadNoiseVarAfterGain;

    // While each row has to read out, reset, and then expose, the (reset +
    // expose) sequence can be overlapped by other row readouts, so the final
    // minimum frame duration is purely a function of row readout time, at least
    // if there's a reasonable number of rows.
    static const nsecs_t kRowReadoutTime;

    static const int32_t kSensitivityRange[2];
    static const uint32_t kDefaultSensitivity;

  private:
    Mutex mControlMutex; // Lock before accessing control parameters
    // Start of control parameters
    Condition mVSync;
    bool      mGotVSync;
    uint64_t  mExposureTime;
    uint64_t  mFrameDuration;
    uint32_t  mGainFactor;
    Buffers  *mNextBuffers;
    uint32_t  mFrameNumber;

    // End of control parameters

    Mutex mReadoutMutex; // Lock before accessing readout variables
    // Start of readout variables
    Condition mReadoutAvailable;
    Condition mReadoutComplete;
    Buffers  *mCapturedBuffers;
    nsecs_t   mCaptureTime;
    SensorListener *mListener;
    // End of readout variables

    // Time of sensor startup, used for simulation zero-time point
    nsecs_t mStartupTime;

    /**
     * Inherited Thread virtual overrides, and members only used by the
     * processing thread
     */
  private:
    virtual status_t readyToRun();

    virtual bool threadLoop();

    nsecs_t mNextCaptureTime;
    Buffers *mNextCapturedBuffers;

    Scene mScene;

    void captureRaw(uint8_t *img, uint32_t gain, uint32_t stride);
    void captureRGBA(uint8_t *img, uint32_t gain, uint32_t stride);
    void captureRGB(uint8_t *img, uint32_t gain, uint32_t stride);
    void captureNV21(uint8_t *img, uint32_t gain, uint32_t stride);
    void captureDepth(uint8_t *img, uint32_t gain, uint32_t stride);
    void captureDepthCloud(uint8_t *img);

};

}

#endif // HW_EMULATOR_CAMERA2_SENSOR_H
