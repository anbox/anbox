/*
* Copyright (C) 2016 The Android Open Source Project
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

#pragma once

#include "ChecksumCalculator.h"

// ChecksumCalculatorThreadInfo is the class that makes ChecksumCalculator
// thread-safe. On the host, please only use ChecksumCalculator through this
// class.

class ChecksumCalculatorThreadInfo {
public:
    ChecksumCalculatorThreadInfo();
    ~ChecksumCalculatorThreadInfo();

    static uint32_t getVersion();
    static bool setVersion(uint32_t version);

    static uint32_t getMaxVersion();
    static const char* getMaxVersionString() {
        return ChecksumCalculator::getMaxVersionStr();
    }

    static size_t checksumByteSize();
    static bool writeChecksum(void* buf,
                              size_t bufLen,
                              void* outputChecksum,
                              size_t outputChecksumLen);

    static bool validate(void* buf,
                         size_t bufLen,
                         void* checksum,
                         size_t checksumLen);
    static void validOrDie(void* buf,
                           size_t bufLen,
                           void* checksum,
                           size_t checksumLen,
                           const char* message);

private:
    ChecksumCalculator m_protocol;
};
