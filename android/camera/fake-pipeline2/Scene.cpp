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

//#define LOG_NDEBUG 0
#define LOG_TAG "EmulatedCamera_Scene"
#include <utils/Log.h>
#include <stdlib.h>
#include <cmath>
#include "Scene.h"

// TODO: This should probably be done host-side in OpenGL for speed and better
// quality

namespace android {

// Define single-letter shortcuts for scene definition, for directly indexing
// mCurrentColors
#define G (Scene::GRASS * Scene::NUM_CHANNELS)
#define S (Scene::GRASS_SHADOW * Scene::NUM_CHANNELS)
#define H (Scene::HILL * Scene::NUM_CHANNELS)
#define W (Scene::WALL * Scene::NUM_CHANNELS)
#define R (Scene::ROOF * Scene::NUM_CHANNELS)
#define D (Scene::DOOR * Scene::NUM_CHANNELS)
#define C (Scene::CHIMNEY * Scene::NUM_CHANNELS)
#define I (Scene::WINDOW * Scene::NUM_CHANNELS)
#define U (Scene::SUN * Scene::NUM_CHANNELS)
#define K (Scene::SKY * Scene::NUM_CHANNELS)
#define M (Scene::MOON * Scene::NUM_CHANNELS)

const int Scene::kSceneWidth = 20;
const int Scene::kSceneHeight = 20;

const uint8_t Scene::kScene[Scene::kSceneWidth * Scene::kSceneHeight] = {
    //      5         10        15        20
    K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
    K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
    K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
    K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
    K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K, // 5
    K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,K,
    K,K,K,K,K,K,K,K,H,H,H,H,H,H,H,H,H,H,H,H,
    K,K,K,K,K,K,K,K,H,H,H,H,H,H,H,C,C,H,H,H,
    K,K,K,K,K,K,H,H,H,H,H,H,H,H,H,C,C,H,H,H,
    H,K,K,K,K,K,H,R,R,R,R,R,R,R,R,R,R,R,R,H, // 10
    H,K,K,K,K,H,H,R,R,R,R,R,R,R,R,R,R,R,R,H,
    H,H,H,K,K,H,H,R,R,R,R,R,R,R,R,R,R,R,R,H,
    H,H,H,K,K,H,H,H,W,W,W,W,W,W,W,W,W,W,H,H,
    S,S,S,G,G,S,S,S,W,W,W,W,W,W,W,W,W,W,S,S,
    S,G,G,G,G,S,S,S,W,I,I,W,D,D,W,I,I,W,S,S, // 15
    G,G,G,G,G,G,S,S,W,I,I,W,D,D,W,I,I,W,S,S,
    G,G,G,G,G,G,G,G,W,W,W,W,D,D,W,W,W,W,G,G,
    G,G,G,G,G,G,G,G,W,W,W,W,D,D,W,W,W,W,G,G,
    G,G,G,G,G,G,G,G,S,S,S,S,S,S,S,S,S,S,G,G,
    G,G,G,G,G,G,G,G,S,S,S,S,S,S,S,S,S,S,G,G, // 20
    //      5         10        15        20
};

#undef G
#undef S
#undef H
#undef W
#undef R
#undef D
#undef C
#undef I
#undef U
#undef K
#undef M

Scene::Scene(
    int sensorWidthPx,
    int sensorHeightPx,
    float sensorSensitivity):
        mSensorWidth(sensorWidthPx),
        mSensorHeight(sensorHeightPx),
        mHour(12),
        mExposureDuration(0.033f),
        mSensorSensitivity(sensorSensitivity)
{
    // Map scene to sensor pixels
    if (mSensorWidth > mSensorHeight) {
        mMapDiv = (mSensorWidth / (kSceneWidth + 1) ) + 1;
    } else {
        mMapDiv = (mSensorHeight / (kSceneHeight + 1) ) + 1;
    }
    mOffsetX = (kSceneWidth * mMapDiv - mSensorWidth) / 2;
    mOffsetY = (kSceneHeight * mMapDiv - mSensorHeight) / 2;

    // Assume that sensor filters are sRGB primaries to start
    mFilterR[0]  =  3.2406f; mFilterR[1]  = -1.5372f; mFilterR[2]  = -0.4986f;
    mFilterGr[0] = -0.9689f; mFilterGr[1] =  1.8758f; mFilterGr[2] =  0.0415f;
    mFilterGb[0] = -0.9689f; mFilterGb[1] =  1.8758f; mFilterGb[2] =  0.0415f;
    mFilterB[0]  =  0.0557f; mFilterB[1]  = -0.2040f; mFilterB[2]  =  1.0570f;


}

Scene::~Scene() {
}

void Scene::setColorFilterXYZ(
        float rX, float rY, float rZ,
        float grX, float grY, float grZ,
        float gbX, float gbY, float gbZ,
        float bX, float bY, float bZ) {
    mFilterR[0]  = rX;  mFilterR[1]  = rY;  mFilterR[2]  = rZ;
    mFilterGr[0] = grX; mFilterGr[1] = grY; mFilterGr[2] = grZ;
    mFilterGb[0] = gbX; mFilterGb[1] = gbY; mFilterGb[2] = gbZ;
    mFilterB[0]  = bX;  mFilterB[1]  = bY;  mFilterB[2]  = bZ;
}

void Scene::setHour(int hour) {
    ALOGV("Hour set to: %d", hour);
    mHour = hour % 24;
}

int Scene::getHour() {
    return mHour;
}

void Scene::setExposureDuration(float seconds) {
    mExposureDuration = seconds;
}

void Scene::calculateScene(nsecs_t time) {
    // Calculate time fractions for interpolation
    int timeIdx = mHour / kTimeStep;
    int nextTimeIdx = (timeIdx + 1) % (24 / kTimeStep);
    const nsecs_t kOneHourInNsec = 1e9 * 60 * 60;
    nsecs_t timeSinceIdx = (mHour - timeIdx * kTimeStep) * kOneHourInNsec + time;
    float timeFrac = timeSinceIdx / (float)(kOneHourInNsec * kTimeStep);

    // Determine overall sunlight levels
    float sunLux =
            kSunlight[timeIdx] * (1 - timeFrac) +
            kSunlight[nextTimeIdx] * timeFrac;
    ALOGV("Sun lux: %f", sunLux);

    float sunShadeLux = sunLux * (kDaylightShadeIllum / kDirectSunIllum);

    // Determine sun/shade illumination chromaticity
    float currentSunXY[2];
    float currentShadeXY[2];

    const float *prevSunXY, *nextSunXY;
    const float *prevShadeXY, *nextShadeXY;
    if (kSunlight[timeIdx] == kSunsetIllum ||
            kSunlight[timeIdx] == kTwilightIllum) {
        prevSunXY = kSunsetXY;
        prevShadeXY = kSunsetXY;
    } else {
        prevSunXY = kDirectSunlightXY;
        prevShadeXY = kDaylightXY;
    }
    if (kSunlight[nextTimeIdx] == kSunsetIllum ||
            kSunlight[nextTimeIdx] == kTwilightIllum) {
        nextSunXY = kSunsetXY;
        nextShadeXY = kSunsetXY;
    } else {
        nextSunXY = kDirectSunlightXY;
        nextShadeXY = kDaylightXY;
    }
    currentSunXY[0] = prevSunXY[0] * (1 - timeFrac) +
            nextSunXY[0] * timeFrac;
    currentSunXY[1] = prevSunXY[1] * (1 - timeFrac) +
            nextSunXY[1] * timeFrac;

    currentShadeXY[0] = prevShadeXY[0] * (1 - timeFrac) +
            nextShadeXY[0] * timeFrac;
    currentShadeXY[1] = prevShadeXY[1] * (1 - timeFrac) +
            nextShadeXY[1] * timeFrac;

    ALOGV("Sun XY: %f, %f, Shade XY: %f, %f",
            currentSunXY[0], currentSunXY[1],
            currentShadeXY[0], currentShadeXY[1]);

    // Converting for xyY to XYZ:
    // X = Y / y * x
    // Y = Y
    // Z = Y / y * (1 - x - y);
    float sunXYZ[3] = {
        sunLux / currentSunXY[1] * currentSunXY[0],
        sunLux,
        sunLux / currentSunXY[1] *
        (1 - currentSunXY[0] - currentSunXY[1])
    };
    float sunShadeXYZ[3] = {
        sunShadeLux / currentShadeXY[1] * currentShadeXY[0],
        sunShadeLux,
        sunShadeLux / currentShadeXY[1] *
        (1 - currentShadeXY[0] - currentShadeXY[1])
    };
    ALOGV("Sun XYZ: %f, %f, %f",
            sunXYZ[0], sunXYZ[1], sunXYZ[2]);
    ALOGV("Sun shade XYZ: %f, %f, %f",
            sunShadeXYZ[0], sunShadeXYZ[1], sunShadeXYZ[2]);

    // Determine moonlight levels
    float moonLux =
            kMoonlight[timeIdx] * (1 - timeFrac) +
            kMoonlight[nextTimeIdx] * timeFrac;
    float moonShadeLux = moonLux * (kDaylightShadeIllum / kDirectSunIllum);

    float moonXYZ[3] = {
        moonLux / kMoonlightXY[1] * kMoonlightXY[0],
        moonLux,
        moonLux / kMoonlightXY[1] *
        (1 - kMoonlightXY[0] - kMoonlightXY[1])
    };
    float moonShadeXYZ[3] = {
        moonShadeLux / kMoonlightXY[1] * kMoonlightXY[0],
        moonShadeLux,
        moonShadeLux / kMoonlightXY[1] *
        (1 - kMoonlightXY[0] - kMoonlightXY[1])
    };

    // Determine starlight level
    const float kClearNightXYZ[3] = {
        kClearNightIllum / kMoonlightXY[1] * kMoonlightXY[0],
        kClearNightIllum,
        kClearNightIllum / kMoonlightXY[1] *
            (1 - kMoonlightXY[0] - kMoonlightXY[1])
    };

    // Calculate direct and shaded light
    float directIllumXYZ[3] = {
        sunXYZ[0] + moonXYZ[0] + kClearNightXYZ[0],
        sunXYZ[1] + moonXYZ[1] + kClearNightXYZ[1],
        sunXYZ[2] + moonXYZ[2] + kClearNightXYZ[2],
    };

    float shadeIllumXYZ[3] = {
        kClearNightXYZ[0],
        kClearNightXYZ[1],
        kClearNightXYZ[2]
    };

    shadeIllumXYZ[0] += (mHour < kSunOverhead) ? sunXYZ[0] : sunShadeXYZ[0];
    shadeIllumXYZ[1] += (mHour < kSunOverhead) ? sunXYZ[1] : sunShadeXYZ[1];
    shadeIllumXYZ[2] += (mHour < kSunOverhead) ? sunXYZ[2] : sunShadeXYZ[2];

    // Moon up period covers 23->0 transition, shift for simplicity
    int adjHour = (mHour + 12) % 24;
    int adjMoonOverhead = (kMoonOverhead + 12 ) % 24;
    shadeIllumXYZ[0] += (adjHour < adjMoonOverhead) ?
            moonXYZ[0] : moonShadeXYZ[0];
    shadeIllumXYZ[1] += (adjHour < adjMoonOverhead) ?
            moonXYZ[1] : moonShadeXYZ[1];
    shadeIllumXYZ[2] += (adjHour < adjMoonOverhead) ?
            moonXYZ[2] : moonShadeXYZ[2];

    ALOGV("Direct XYZ: %f, %f, %f",
            directIllumXYZ[0],directIllumXYZ[1],directIllumXYZ[2]);
    ALOGV("Shade XYZ: %f, %f, %f",
            shadeIllumXYZ[0], shadeIllumXYZ[1], shadeIllumXYZ[2]);

    for (int i = 0; i < NUM_MATERIALS; i++) {
        // Converting for xyY to XYZ:
        // X = Y / y * x
        // Y = Y
        // Z = Y / y * (1 - x - y);
        float matXYZ[3] = {
            kMaterials_xyY[i][2] / kMaterials_xyY[i][1] *
              kMaterials_xyY[i][0],
            kMaterials_xyY[i][2],
            kMaterials_xyY[i][2] / kMaterials_xyY[i][1] *
              (1 - kMaterials_xyY[i][0] - kMaterials_xyY[i][1])
        };

        if (kMaterialsFlags[i] == 0 || kMaterialsFlags[i] & kSky) {
            matXYZ[0] *= directIllumXYZ[0];
            matXYZ[1] *= directIllumXYZ[1];
            matXYZ[2] *= directIllumXYZ[2];
        } else if (kMaterialsFlags[i] & kShadowed) {
            matXYZ[0] *= shadeIllumXYZ[0];
            matXYZ[1] *= shadeIllumXYZ[1];
            matXYZ[2] *= shadeIllumXYZ[2];
        } // else if (kMaterialsFlags[i] * kSelfLit), do nothing

        ALOGV("Mat %d XYZ: %f, %f, %f", i, matXYZ[0], matXYZ[1], matXYZ[2]);
        float luxToElectrons = mSensorSensitivity * mExposureDuration /
                (kAperture * kAperture);
        mCurrentColors[i*NUM_CHANNELS + 0] =
                (mFilterR[0] * matXYZ[0] +
                 mFilterR[1] * matXYZ[1] +
                 mFilterR[2] * matXYZ[2])
                * luxToElectrons;
        mCurrentColors[i*NUM_CHANNELS + 1] =
                (mFilterGr[0] * matXYZ[0] +
                 mFilterGr[1] * matXYZ[1] +
                 mFilterGr[2] * matXYZ[2])
                * luxToElectrons;
        mCurrentColors[i*NUM_CHANNELS + 2] =
                (mFilterGb[0] * matXYZ[0] +
                 mFilterGb[1] * matXYZ[1] +
                 mFilterGb[2] * matXYZ[2])
                * luxToElectrons;
        mCurrentColors[i*NUM_CHANNELS + 3] =
                (mFilterB[0] * matXYZ[0] +
                 mFilterB[1] * matXYZ[1] +
                 mFilterB[2] * matXYZ[2])
                * luxToElectrons;

        ALOGV("Color %d RGGB: %d, %d, %d, %d", i,
                mCurrentColors[i*NUM_CHANNELS + 0],
                mCurrentColors[i*NUM_CHANNELS + 1],
                mCurrentColors[i*NUM_CHANNELS + 2],
                mCurrentColors[i*NUM_CHANNELS + 3]);
    }
    // Shake viewpoint; horizontal and vertical sinusoids at roughly
    // human handshake frequencies
    mHandshakeX =
            ( kFreq1Magnitude * std::sin(kHorizShakeFreq1 * timeSinceIdx) +
              kFreq2Magnitude * std::sin(kHorizShakeFreq2 * timeSinceIdx) ) *
            mMapDiv * kShakeFraction;

    mHandshakeY =
            ( kFreq1Magnitude * std::sin(kVertShakeFreq1 * timeSinceIdx) +
              kFreq2Magnitude * std::sin(kVertShakeFreq2 * timeSinceIdx) ) *
            mMapDiv * kShakeFraction;

    // Set starting pixel
    setReadoutPixel(0,0);
}

void Scene::setReadoutPixel(int x, int y) {
    mCurrentX = x;
    mCurrentY = y;
    mSubX = (x + mOffsetX + mHandshakeX) % mMapDiv;
    mSubY = (y + mOffsetY + mHandshakeY) % mMapDiv;
    mSceneX = (x + mOffsetX + mHandshakeX) / mMapDiv;
    mSceneY = (y + mOffsetY + mHandshakeY) / mMapDiv;
    mSceneIdx = mSceneY * kSceneWidth + mSceneX;
    mCurrentSceneMaterial = &(mCurrentColors[kScene[mSceneIdx]]);
}

const uint32_t* Scene::getPixelElectrons() {
    const uint32_t *pixel = mCurrentSceneMaterial;
    mCurrentX++;
    mSubX++;
    if (mCurrentX >= mSensorWidth) {
        mCurrentX = 0;
        mCurrentY++;
        if (mCurrentY >= mSensorHeight) mCurrentY = 0;
        setReadoutPixel(mCurrentX, mCurrentY);
    } else if (mSubX > mMapDiv) {
        mSceneIdx++;
        mSceneX++;
        mCurrentSceneMaterial = &(mCurrentColors[kScene[mSceneIdx]]);
        mSubX = 0;
    }
    return pixel;
}

// Handshake model constants.
// Frequencies measured in a nanosecond timebase
const float Scene::kHorizShakeFreq1 = 2 * M_PI * 2  / 1e9; // 2 Hz
const float Scene::kHorizShakeFreq2 = 2 * M_PI * 13 / 1e9; // 13 Hz
const float Scene::kVertShakeFreq1  = 2 * M_PI * 3  / 1e9; // 3 Hz
const float Scene::kVertShakeFreq2  = 2 * M_PI * 11 / 1e9; // 1 Hz
const float Scene::kFreq1Magnitude  = 5;
const float Scene::kFreq2Magnitude  = 1;
const float Scene::kShakeFraction   = 0.03; // As a fraction of a scene tile

// RGB->YUV, Jpeg standard
const float Scene::kRgb2Yuv[12] = {
       0.299f,    0.587f,    0.114f,    0.f,
    -0.16874f, -0.33126f,      0.5f, -128.f,
         0.5f, -0.41869f, -0.08131f, -128.f,
};

// Aperture of imaging lens
const float Scene::kAperture = 2.8;

// Sun illumination levels through the day
const float Scene::kSunlight[24/kTimeStep] =
{
    0, // 00:00
    0,
    0,
    kTwilightIllum, // 06:00
    kDirectSunIllum,
    kDirectSunIllum,
    kDirectSunIllum, // 12:00
    kDirectSunIllum,
    kDirectSunIllum,
    kSunsetIllum, // 18:00
    kTwilightIllum,
    0
};

// Moon illumination levels through the day
const float Scene::kMoonlight[24/kTimeStep] =
{
    kFullMoonIllum, // 00:00
    kFullMoonIllum,
    0,
    0, // 06:00
    0,
    0,
    0, // 12:00
    0,
    0,
    0, // 18:00
    0,
    kFullMoonIllum
};

const int Scene::kSunOverhead = 12;
const int Scene::kMoonOverhead = 0;

// Used for sun illumination levels
const float Scene::kDirectSunIllum     = 100000;
const float Scene::kSunsetIllum        = 400;
const float Scene::kTwilightIllum      = 4;
// Used for moon illumination levels
const float Scene::kFullMoonIllum      = 1;
// Other illumination levels
const float Scene::kDaylightShadeIllum = 20000;
const float Scene::kClearNightIllum    = 2e-3;
const float Scene::kStarIllum          = 2e-6;
const float Scene::kLivingRoomIllum    = 50;

const float Scene::kIncandescentXY[2]   = { 0.44757f, 0.40745f};
const float Scene::kDirectSunlightXY[2] = { 0.34842f, 0.35161f};
const float Scene::kDaylightXY[2]       = { 0.31271f, 0.32902f};
const float Scene::kNoonSkyXY[2]        = { 0.346f,   0.359f};
const float Scene::kMoonlightXY[2]      = { 0.34842f, 0.35161f};
const float Scene::kSunsetXY[2]         = { 0.527f,   0.413f};

const uint8_t Scene::kSelfLit  = 0x01;
const uint8_t Scene::kShadowed = 0x02;
const uint8_t Scene::kSky      = 0x04;

// For non-self-lit materials, the Y component is normalized with 1=full
// reflectance; for self-lit materials, it's the constant illuminance in lux.
const float Scene::kMaterials_xyY[Scene::NUM_MATERIALS][3] = {
    { 0.3688f, 0.4501f, .1329f }, // GRASS
    { 0.3688f, 0.4501f, .1329f }, // GRASS_SHADOW
    { 0.3986f, 0.5002f, .4440f }, // HILL
    { 0.3262f, 0.5040f, .2297f }, // WALL
    { 0.4336f, 0.3787f, .1029f }, // ROOF
    { 0.3316f, 0.2544f, .0639f }, // DOOR
    { 0.3425f, 0.3577f, .0887f }, // CHIMNEY
    { kIncandescentXY[0], kIncandescentXY[1], kLivingRoomIllum }, // WINDOW
    { kDirectSunlightXY[0], kDirectSunlightXY[1], kDirectSunIllum }, // SUN
    { kNoonSkyXY[0], kNoonSkyXY[1], kDaylightShadeIllum / kDirectSunIllum }, // SKY
    { kMoonlightXY[0], kMoonlightXY[1], kFullMoonIllum } // MOON
};

const uint8_t Scene::kMaterialsFlags[Scene::NUM_MATERIALS] = {
    0,
    kShadowed,
    kShadowed,
    kShadowed,
    kShadowed,
    kShadowed,
    kShadowed,
    kSelfLit,
    kSelfLit,
    kSky,
    kSelfLit,
};

} // namespace android
