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
 * The Scene class implements a simple physical simulation of a scene, using the
 * CIE 1931 colorspace to represent light in physical units (lux).
 *
 * It's fairly approximate, but does provide a scene with realistic widely
 * variable illumination levels and colors over time.
 *
 */

#ifndef HW_EMULATOR_CAMERA2_SCENE_H
#define HW_EMULATOR_CAMERA2_SCENE_H

#include "utils/Timers.h"

namespace android {

class Scene {
  public:
    Scene(int sensorWidthPx,
            int sensorHeightPx,
            float sensorSensitivity);
    ~Scene();

    // Set the filter coefficients for the red, green, and blue filters on the
    // sensor. Used as an optimization to pre-calculate various illuminance
    // values. Two different green filters can be provided, to account for
    // possible cross-talk on a Bayer sensor. Must be called before
    // calculateScene.
    void setColorFilterXYZ(
        float rX, float rY, float rZ,
        float grX, float grY, float grZ,
        float gbX, float gbY, float gbZ,
        float bX, float bY, float bZ);

    // Set time of day (24-hour clock). This controls the general light levels
    // in the scene. Must be called before calculateScene
    void setHour(int hour);
    // Get current hour
    int getHour();

    // Set the duration of exposure for determining luminous exposure.
    // Must be called before calculateScene
    void setExposureDuration(float seconds);

    // Calculate scene information for current hour and the time offset since
    // the hour. Must be called at least once before calling getLuminousExposure.
    // Resets pixel readout location to 0,0
    void calculateScene(nsecs_t time);

    // Set sensor pixel readout location.
    void setReadoutPixel(int x, int y);

    // Get sensor response in physical units (electrons) for light hitting the
    // current readout pixel, after passing through color filters. The readout
    // pixel will be auto-incremented. The returned array can be indexed with
    // ColorChannels.
    const uint32_t* getPixelElectrons();

    enum ColorChannels {
        R = 0,
        Gr,
        Gb,
        B,
        Y,
        Cb,
        Cr,
        NUM_CHANNELS
    };

  private:
    // Sensor color filtering coefficients in XYZ
    float mFilterR[3];
    float mFilterGr[3];
    float mFilterGb[3];
    float mFilterB[3];

    int mOffsetX, mOffsetY;
    int mMapDiv;

    int mHandshakeX, mHandshakeY;

    int mSensorWidth;
    int mSensorHeight;
    int mCurrentX;
    int mCurrentY;
    int mSubX;
    int mSubY;
    int mSceneX;
    int mSceneY;
    int mSceneIdx;
    uint32_t *mCurrentSceneMaterial;

    int mHour;
    float mExposureDuration;
    float mSensorSensitivity;

    enum Materials {
        GRASS = 0,
        GRASS_SHADOW,
        HILL,
        WALL,
        ROOF,
        DOOR,
        CHIMNEY,
        WINDOW,
        SUN,
        SKY,
        MOON,
        NUM_MATERIALS
    };

    uint32_t mCurrentColors[NUM_MATERIALS*NUM_CHANNELS];

    /**
     * Constants for scene definition. These are various degrees of approximate.
     */

    // Fake handshake parameters. Two shake frequencies per axis, plus magnitude
    // as a fraction of a scene tile, and relative magnitudes for the frequencies
    static const float kHorizShakeFreq1;
    static const float kHorizShakeFreq2;
    static const float kVertShakeFreq1;
    static const float kVertShakeFreq2;
    static const float kFreq1Magnitude;
    static const float kFreq2Magnitude;

    static const float kShakeFraction;

    // RGB->YUV conversion
    static const float kRgb2Yuv[12];

    // Aperture of imaging lens
    static const float kAperture;

    // Sun, moon illuminance levels in 2-hour increments. These don't match any
    // real day anywhere.
    static const uint32_t kTimeStep = 2;
    static const float kSunlight[];
    static const float kMoonlight[];
    static const int kSunOverhead;
    static const int kMoonOverhead;

    // Illumination levels for various conditions, in lux
    static const float kDirectSunIllum;
    static const float kDaylightShadeIllum;
    static const float kSunsetIllum;
    static const float kTwilightIllum;
    static const float kFullMoonIllum;
    static const float kClearNightIllum;
    static const float kStarIllum;
    static const float kLivingRoomIllum;

    // Chromaticity of various illumination sources
    static const float kIncandescentXY[2];
    static const float kDirectSunlightXY[2];
    static const float kDaylightXY[2];
    static const float kNoonSkyXY[2];
    static const float kMoonlightXY[2];
    static const float kSunsetXY[2];

    static const uint8_t kSelfLit;
    static const uint8_t kShadowed;
    static const uint8_t kSky;

    static const float kMaterials_xyY[NUM_MATERIALS][3];
    static const uint8_t kMaterialsFlags[NUM_MATERIALS];

    static const int kSceneWidth;
    static const int kSceneHeight;
    static const uint8_t kScene[];
};

}

#endif // HW_EMULATOR_CAMERA2_SCENE_H
