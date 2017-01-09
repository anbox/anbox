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

#ifndef HW_EMULATOR_CAMERA_CONVERTERS_H
#define HW_EMULATOR_CAMERA_CONVERTERS_H

#include <endian.h>

#ifndef __BYTE_ORDER
#error "could not determine byte order"
#endif

/*
 * Contains declaration of framebuffer conversion routines.
 *
 * NOTE: RGB and big/little endian considerations. Wherewer in this code RGB
 * pixels are represented as WORD, or DWORD, the color order inside the
 * WORD / DWORD matches the one that would occur if that WORD / DWORD would have
 * been read from the typecasted framebuffer:
 *
 *      const uint32_t rgb = *reinterpret_cast<const uint32_t*>(framebuffer);
 *
 * So, if this code runs on the little endian CPU, red color in 'rgb' would be
 * masked as 0x000000ff, and blue color would be masked as 0x00ff0000, while if
 * the code runs on a big endian CPU, the red color in 'rgb' would be masked as
 * 0xff000000, and blue color would be masked as 0x0000ff00,
 */

namespace android {

/*
 * RGB565 color masks
 */

#if __BYTE_ORDER == __LITTLE_ENDIAN
static const uint16_t kRed5     = 0x001f;
static const uint16_t kGreen6   = 0x07e0;
static const uint16_t kBlue5    = 0xf800;
#else   // __BYTE_ORDER
static const uint16_t kRed5     = 0xf800;
static const uint16_t kGreen6   = 0x07e0;
static const uint16_t kBlue5    = 0x001f;
#endif  // __BYTE_ORDER
static const uint32_t kBlack16  = 0x0000;
static const uint32_t kWhite16  = kRed5 | kGreen6 | kBlue5;

/*
 * RGB32 color masks
 */

#if __BYTE_ORDER == __LITTLE_ENDIAN
static const uint32_t kRed8     = 0x000000ff;
static const uint32_t kGreen8   = 0x0000ff00;
static const uint32_t kBlue8    = 0x00ff0000;
#else   // __BYTE_ORDER
static const uint32_t kRed8     = 0x00ff0000;
static const uint32_t kGreen8   = 0x0000ff00;
static const uint32_t kBlue8    = 0x000000ff;
#endif  // __BYTE_ORDER
static const uint32_t kBlack32  = 0x00000000;
static const uint32_t kWhite32  = kRed8 | kGreen8 | kBlue8;

/*
 * Extracting, and saving color bytes from / to WORD / DWORD RGB.
 */

#if __BYTE_ORDER == __LITTLE_ENDIAN
/* Extract red, green, and blue bytes from RGB565 word. */
#define R16(rgb)    static_cast<uint8_t>(rgb & kRed5)
#define G16(rgb)    static_cast<uint8_t>((rgb & kGreen6) >> 5)
#define B16(rgb)    static_cast<uint8_t>((rgb & kBlue5) >> 11)
/* Make 8 bits red, green, and blue, extracted from RGB565 word. */
#define R16_32(rgb) static_cast<uint8_t>(((rgb & kRed5) << 3) | ((rgb & kRed5) >> 2))
#define G16_32(rgb) static_cast<uint8_t>(((rgb & kGreen6) >> 3) | ((rgb & kGreen6) >> 9))
#define B16_32(rgb) static_cast<uint8_t>(((rgb & kBlue5) >> 8) | ((rgb & kBlue5) >> 14))
/* Extract red, green, and blue bytes from RGB32 dword. */
#define R32(rgb)    static_cast<uint8_t>(rgb & kRed8)
#define G32(rgb)    static_cast<uint8_t>(((rgb & kGreen8) >> 8) & 0xff)
#define B32(rgb)    static_cast<uint8_t>(((rgb & kBlue8) >> 16) & 0xff)
/* Build RGB565 word from red, green, and blue bytes. */
#define RGB565(r, g, b) static_cast<uint16_t>((((static_cast<uint16_t>(b) << 6) | g) << 5) | r)
/* Build RGB32 dword from red, green, and blue bytes. */
#define RGB32(r, g, b) static_cast<uint32_t>((((static_cast<uint32_t>(b) << 8) | g) << 8) | r)
#else   // __BYTE_ORDER
/* Extract red, green, and blue bytes from RGB565 word. */
#define R16(rgb)    static_cast<uint8_t>((rgb & kRed5) >> 11)
#define G16(rgb)    static_cast<uint8_t>((rgb & kGreen6) >> 5)
#define B16(rgb)    static_cast<uint8_t>(rgb & kBlue5)
/* Make 8 bits red, green, and blue, extracted from RGB565 word. */
#define R16_32(rgb) static_cast<uint8_t>(((rgb & kRed5) >> 8) | ((rgb & kRed5) >> 14))
#define G16_32(rgb) static_cast<uint8_t>(((rgb & kGreen6) >> 3) | ((rgb & kGreen6) >> 9))
#define B16_32(rgb) static_cast<uint8_t>(((rgb & kBlue5) << 3) | ((rgb & kBlue5) >> 2))
/* Extract red, green, and blue bytes from RGB32 dword. */
#define R32(rgb)    static_cast<uint8_t>((rgb & kRed8) >> 16)
#define G32(rgb)    static_cast<uint8_t>((rgb & kGreen8) >> 8)
#define B32(rgb)    static_cast<uint8_t>(rgb & kBlue8)
/* Build RGB565 word from red, green, and blue bytes. */
#define RGB565(r, g, b) static_cast<uint16_t>((((static_cast<uint16_t>(r) << 6) | g) << 5) | b)
/* Build RGB32 dword from red, green, and blue bytes. */
#define RGB32(r, g, b) static_cast<uint32_t>((((static_cast<uint32_t>(r) << 8) | g) << 8) | b)
#endif  // __BYTE_ORDER

/* An union that simplifies breaking 32 bit RGB into separate R, G, and B colors.
 */
typedef union RGB32_t {
    uint32_t    color;
    struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        uint8_t r; uint8_t g; uint8_t b; uint8_t a;
#else   // __BYTE_ORDER
        uint8_t a; uint8_t b; uint8_t g; uint8_t r;
#endif  // __BYTE_ORDER
    };
} RGB32_t;


/* Clips a value to the unsigned 0-255 range, treating negative values as zero.
 */
static __inline__ int
clamp(int x)
{
    if (x > 255) return 255;
    if (x < 0)   return 0;
    return x;
}

/********************************************************************************
 * Basics of RGB -> YUV conversion
 *******************************************************************************/

/*
 * RGB -> YUV conversion macros
 */
#define RGB2Y(r, g, b) (uint8_t)(((66 * (r) + 129 * (g) +  25 * (b) + 128) >> 8) +  16)
#define RGB2U(r, g, b) (uint8_t)(((-38 * (r) - 74 * (g) + 112 * (b) + 128) >> 8) + 128)
#define RGB2V(r, g, b) (uint8_t)(((112 * (r) - 94 * (g) -  18 * (b) + 128) >> 8) + 128)

/* Converts R8 G8 B8 color to YUV. */
static __inline__ void
R8G8B8ToYUV(uint8_t r, uint8_t g, uint8_t b, uint8_t* y, uint8_t* u, uint8_t* v)
{
    *y = RGB2Y((int)r, (int)g, (int)b);
    *u = RGB2U((int)r, (int)g, (int)b);
    *v = RGB2V((int)r, (int)g, (int)b);
}

/* Converts RGB565 color to YUV. */
static __inline__ void
RGB565ToYUV(uint16_t rgb, uint8_t* y, uint8_t* u, uint8_t* v)
{
    R8G8B8ToYUV(R16_32(rgb), G16_32(rgb), B16_32(rgb), y, u, v);
}

/* Converts RGB32 color to YUV. */
static __inline__ void
RGB32ToYUV(uint32_t rgb, uint8_t* y, uint8_t* u, uint8_t* v)
{
    RGB32_t rgb_c;
    rgb_c.color = rgb;
    R8G8B8ToYUV(rgb_c.r, rgb_c.g, rgb_c.b, y, u, v);
}

/********************************************************************************
 * Basics of YUV -> RGB conversion.
 * Note that due to the fact that guest uses RGB only on preview window, and the
 * RGB format that is used is RGB565, we can limit YUV -> RGB conversions to
 * RGB565 only.
 *******************************************************************************/

/*
 * YUV -> RGB conversion macros
 */

/* "Optimized" macros that take specialy prepared Y, U, and V values:
 *  C = Y - 16
 *  D = U - 128
 *  E = V - 128
 */
#define YUV2RO(C, D, E) clamp((298 * (C) + 409 * (E) + 128) >> 8)
#define YUV2GO(C, D, E) clamp((298 * (C) - 100 * (D) - 208 * (E) + 128) >> 8)
#define YUV2BO(C, D, E) clamp((298 * (C) + 516 * (D) + 128) >> 8)

/*
 *  Main macros that take the original Y, U, and V values
 */
#define YUV2R(y, u, v) clamp((298 * ((y)-16) + 409 * ((v)-128) + 128) >> 8)
#define YUV2G(y, u, v) clamp((298 * ((y)-16) - 100 * ((u)-128) - 208 * ((v)-128) + 128) >> 8)
#define YUV2B(y, u, v) clamp((298 * ((y)-16) + 516 * ((u)-128) + 128) >> 8)


/* Converts YUV color to RGB565. */
static __inline__ uint16_t
YUVToRGB565(int y, int u, int v)
{
    /* Calculate C, D, and E values for the optimized macro. */
    y -= 16; u -= 128; v -= 128;
    const uint16_t r = (YUV2RO(y,u,v) >> 3) & 0x1f;
    const uint16_t g = (YUV2GO(y,u,v) >> 2) & 0x3f;
    const uint16_t b = (YUV2BO(y,u,v) >> 3) & 0x1f;
    return RGB565(r, g, b);
}

/* Converts YUV color to RGB32. */
static __inline__ uint32_t
YUVToRGB32(int y, int u, int v)
{
    /* Calculate C, D, and E values for the optimized macro. */
    y -= 16; u -= 128; v -= 128;
    RGB32_t rgb;
    rgb.r = YUV2RO(y,u,v) & 0xff;
    rgb.g = YUV2GO(y,u,v) & 0xff;
    rgb.b = YUV2BO(y,u,v) & 0xff;
    return rgb.color;
}

/* YUV pixel descriptor. */
struct YUVPixel {
    uint8_t     Y;
    uint8_t     U;
    uint8_t     V;

    inline YUVPixel()
        : Y(0), U(0), V(0)
    {
    }

    inline explicit YUVPixel(uint16_t rgb565)
    {
        RGB565ToYUV(rgb565, &Y, &U, &V);
    }

    inline explicit YUVPixel(uint32_t rgb32)
    {
        RGB32ToYUV(rgb32, &Y, &U, &V);
    }

    inline void get(uint8_t* pY, uint8_t* pU, uint8_t* pV) const
    {
        *pY = Y; *pU = U; *pV = V;
    }
};

/* Converts an YV12 framebuffer to RGB565 framebuffer.
 * Param:
 *  yv12 - YV12 framebuffer.
 *  rgb - RGB565 framebuffer.
 *  width, height - Dimensions for both framebuffers.
 */
void YV12ToRGB565(const void* yv12, void* rgb, int width, int height);

/* Converts an YV12 framebuffer to RGB32 framebuffer.
 * Param:
 *  yv12 - YV12 framebuffer.
 *  rgb - RGB32 framebuffer.
 *  width, height - Dimensions for both framebuffers.
 */
void YV12ToRGB32(const void* yv12, void* rgb, int width, int height);

/* Converts an YU12 framebuffer to RGB32 framebuffer.
 * Param:
 *  yu12 - YU12 framebuffer.
 *  rgb - RGB32 framebuffer.
 *  width, height - Dimensions for both framebuffers.
 */
void YU12ToRGB32(const void* yu12, void* rgb, int width, int height);

/* Converts an NV12 framebuffer to RGB565 framebuffer.
 * Param:
 *  nv12 - NV12 framebuffer.
 *  rgb - RGB565 framebuffer.
 *  width, height - Dimensions for both framebuffers.
 */
void NV12ToRGB565(const void* nv12, void* rgb, int width, int height);

/* Converts an NV12 framebuffer to RGB32 framebuffer.
 * Param:
 *  nv12 - NV12 framebuffer.
 *  rgb - RGB32 framebuffer.
 *  width, height - Dimensions for both framebuffers.
 */
void NV12ToRGB32(const void* nv12, void* rgb, int width, int height);

/* Converts an NV21 framebuffer to RGB565 framebuffer.
 * Param:
 *  nv21 - NV21 framebuffer.
 *  rgb - RGB565 framebuffer.
 *  width, height - Dimensions for both framebuffers.
 */
void NV21ToRGB565(const void* nv21, void* rgb, int width, int height);

/* Converts an NV21 framebuffer to RGB32 framebuffer.
 * Param:
 *  nv21 - NV21 framebuffer.
 *  rgb - RGB32 framebuffer.
 *  width, height - Dimensions for both framebuffers.
 */
void NV21ToRGB32(const void* nv21, void* rgb, int width, int height);

}; /* namespace android */

#endif  /* HW_EMULATOR_CAMERA_CONVERTERS_H */
