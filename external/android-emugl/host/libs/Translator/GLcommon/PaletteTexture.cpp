/*
* Copyright (C) 2011 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include "GLcommon/PaletteTexture.h"
#include <stdio.h>



struct Color
{
  Color(unsigned char r, unsigned char g,unsigned char b, unsigned char a):red(r),green(g),blue(b),alpha(a){};
  unsigned char red;
  unsigned char green;
  unsigned char blue;
  unsigned char alpha;
};

void getPaletteInfo(GLenum internalFormat,unsigned int& indexSizeBits,unsigned int& colorSizeBytes,GLenum& colorFrmt) {

        colorFrmt = GL_RGB;
        switch(internalFormat)
    {
    case GL_PALETTE4_RGB8_OES:
        indexSizeBits = 4;
        colorSizeBytes = 3;
        break;

    case GL_PALETTE4_RGBA8_OES:
        indexSizeBits = 4;
        colorSizeBytes = 4;
        colorFrmt = GL_RGBA;
        break;

    case GL_PALETTE4_RGBA4_OES:
    case GL_PALETTE4_RGB5_A1_OES:
        colorFrmt = GL_RGBA;
        /* fall-through */
    case GL_PALETTE4_R5_G6_B5_OES:
        indexSizeBits = 4;
        colorSizeBytes = 2;
        break;

    case GL_PALETTE8_RGB8_OES:
        indexSizeBits = 8;
        colorSizeBytes = 3;
        break;

    case GL_PALETTE8_RGBA8_OES:
        indexSizeBits = 8;
        colorSizeBytes = 4;
        colorFrmt = GL_RGBA;
        break;

    case GL_PALETTE8_RGBA4_OES:
    case GL_PALETTE8_RGB5_A1_OES:
        colorFrmt = GL_RGBA;
        /* fall-through */
    case GL_PALETTE8_R5_G6_B5_OES:
        indexSizeBits = 8;
        colorSizeBytes = 2;
        break;
    }
}


Color paletteColor(const unsigned char* pallete,unsigned int index,GLenum format)
{
        short s;
        switch(format) {
        //RGB
    case GL_PALETTE4_RGB8_OES:
    case GL_PALETTE8_RGB8_OES:
            return Color(pallete[index],pallete[index+1],pallete[index+2],0);
    case GL_PALETTE8_R5_G6_B5_OES:
    case GL_PALETTE4_R5_G6_B5_OES:
            s = *((short *)(pallete+index));
            return Color((s >> 11)*255/31,((s >> 5) & 0x3f)*255/63 ,(s & 0x1f)*255/31,0);

        //RGBA
    case GL_PALETTE4_RGBA8_OES:
    case GL_PALETTE8_RGBA8_OES:
            return Color(pallete[index],pallete[index+1],pallete[index+2],pallete[index+3]);
    case GL_PALETTE4_RGBA4_OES:
    case GL_PALETTE8_RGBA4_OES:
            s = *((short *)(pallete+index));
            return Color(((s >> 12) & 0xf)*255/15,((s >> 8) & 0xf)*255/15,((s >> 4) & 0xf)*255/15 ,(s & 0xf)*255/15);
    case GL_PALETTE4_RGB5_A1_OES:
    case GL_PALETTE8_RGB5_A1_OES:
            s = *((short *)(pallete+index));
            return Color(((s >> 11) & 0x1f)*255/31,((s >> 6) & 0x1f)*255/31,((s >> 1) & 0x1f)*255/31 ,(s & 0x1) * 255);
        default:
            return Color(255,255,255,255);
    }
}

unsigned char* uncompressTexture(GLenum internalformat,GLenum& formatOut,GLsizei width,GLsizei height,GLsizei imageSize, const GLvoid* data,GLint level) {

    unsigned int indexSizeBits;  //the size of the color index in the pallete
    unsigned int colorSizeBytes; //the size of each color cell in the pallete

    getPaletteInfo(internalformat,indexSizeBits,colorSizeBytes,formatOut);
    if(!data)
    {
        return NULL;
    }

    const unsigned char* palette = static_cast<const unsigned char *>(data);

    //the pallete positioned in the begininng of the data
    // so we jump over it to get to the colos indices in the palette

    int nColors = 2 << (indexSizeBits -1); //2^indexSizeBits
    int paletteSizeBytes = nColors*colorSizeBytes;
    const unsigned char* imageIndices =  palette + paletteSizeBytes;

    //jumping to the the correct mipmap level
    for(int i=0;i<level;i++) {
        imageIndices+= (width*height*indexSizeBits)/8;
        width  = width  >> 1;
        height = height >> 1;
    }

    int colorSizeOut = (formatOut == GL_RGB? 3:4);
    int nPixels = width*height;
    unsigned char* pixelsOut = new unsigned char[nPixels*colorSizeOut];
    if(!pixelsOut) return NULL;

    int leftBytes = ((palette + imageSize) /* the end of data pointer*/
                      - imageIndices);
    int leftPixels = (leftBytes * 8 )/indexSizeBits;

    int maxIndices = (leftPixels < nPixels) ? leftPixels:nPixels;

    //filling the pixels array
    for(int i =0 ; i < maxIndices ; i++) {
        int paletteIndex = 0;
        int indexOut = i*colorSizeOut;
        if(indexSizeBits == 4) {
            paletteIndex = (i%2) == 0 ?
                           imageIndices[i/2] >> 4:  //upper bits
                           imageIndices[i/2] & 0xf; //lower bits
        } else {
            paletteIndex = imageIndices[i];
        }

        paletteIndex*=colorSizeBytes;
        Color c = paletteColor(palette,paletteIndex,internalformat);

        pixelsOut[indexOut] = c.red;
        pixelsOut[indexOut+1] = c.green;
        pixelsOut[indexOut+2] = c.blue;
        if(formatOut == GL_RGBA) {
            pixelsOut[indexOut+3] = c.alpha;
        }
    }
    return pixelsOut;
}

