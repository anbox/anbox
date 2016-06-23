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

#include <JNIHelp.h>
#include <jni.h>

#define LOG_TAG "AnboxSupport"

#include <utils/Log.h>

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    ALOGI("Successfully loaded Anbox support library");

    return JNI_VERSION_1_4;
}
