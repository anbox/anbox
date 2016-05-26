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

#include "ChecksumCalculator.h"

#include <string>
#include <vector>
#include <string.h>

// Checklist when implementing new protocol:
// 1. update CHECKSUMHELPER_MAX_VERSION
// 2. update maxChecksumSize()
// 3. update checksumByteSize()
// 4. update addBuffer, writeChecksum, resetChecksum, validate

// change CHECKSUMHELPER_MAX_VERSION when you want to update the protocol version
#define CHECKSUMHELPER_MAX_VERSION 1

// checksum buffer size
// Please add a new checksum buffer size when implementing a new protocol,
// as well as modifying the maxChecksumSize function.
static const size_t kV1ChecksumSize = 8;

static constexpr size_t maxChecksumSize() {
    return 0 > kV1ChecksumSize ? 0 : kV1ChecksumSize;
}

static const size_t kMaxChecksumSize = maxChecksumSize();

// utility macros to create checksum string at compilation time
#define CHECKSUMHELPER_VERSION_STR_PREFIX "ANDROID_EMU_CHECKSUM_HELPER_v"
#define CHECKSUMHELPER_MACRO_TO_STR(x) #x
#define CHECKSUMHELPER_MACRO_VAL_TO_STR(x) CHECKSUMHELPER_MACRO_TO_STR(x)

static const uint32_t kMaxVersion = CHECKSUMHELPER_MAX_VERSION;
static const char* kMaxVersionStrPrefix = CHECKSUMHELPER_VERSION_STR_PREFIX;
static const char* kMaxVersionStr = CHECKSUMHELPER_VERSION_STR_PREFIX CHECKSUMHELPER_MACRO_VAL_TO_STR(CHECKSUMHELPER_MAX_VERSION);

#undef CHECKSUMHELPER_MAX_VERSION
#undef CHECKSUMHELPER_VERSION_STR_PREFIX
#undef CHECKSUMHELPER_MACRO_TO_STR
#undef CHECKSUMHELPER_MACRO_VAL_TO_STR

uint32_t ChecksumCalculator::getMaxVersion() {return kMaxVersion;}
const char* ChecksumCalculator::getMaxVersionStr() {return kMaxVersionStr;}
const char* ChecksumCalculator::getMaxVersionStrPrefix() {return kMaxVersionStrPrefix;}

bool ChecksumCalculator::setVersion(uint32_t version) {
    if (version > kMaxVersion) {  // unsupported version
        LOG_CHECKSUMHELPER("%s: ChecksumCalculator Set Unsupported version Version %d\n",
                __FUNCTION__, m_version);
        return false;
    }
    if (m_isEncodingChecksum) { // setVersion is called in the middle of encoding checksums
        LOG_CHECKSUMHELPER("%s: called between addBuffer and writeChecksum\n",
                __FUNCTION__);
        return false;
    }
    m_version = version;
    LOG_CHECKSUMHELPER("%s: ChecksumCalculator Set Version %d\n", __FUNCTION__,
                m_version);
    return true;
}

size_t ChecksumCalculator::checksumByteSize() const {
    switch (m_version) {
        case 0:
            return 0;
        case 1:
            return sizeof(uint32_t) + sizeof(m_numWrite);
        default:
            return 0;
    }
}

void ChecksumCalculator::addBuffer(const void* buf, size_t packetLen) {
    m_isEncodingChecksum = true;
    switch (m_version) {
        case 1:
            m_v1BufferTotalLength += packetLen;
            break;
    }
}

bool ChecksumCalculator::writeChecksum(void* outputChecksum, size_t outputChecksumLen) {
    if (outputChecksumLen < checksumByteSize()) return false;
    char *checksumPtr = (char *)outputChecksum;
    switch (m_version) {
        case 1: { // protocol v1 is to reverse the packetLen and write it at the end
            uint32_t val = computeV1Checksum();
            memcpy(checksumPtr, &val, sizeof(val));
            memcpy(checksumPtr+sizeof(val), &m_numWrite, sizeof(m_numWrite));
            break;
        }
    }
    resetChecksum();
    m_numWrite++;
    return true;
}

void ChecksumCalculator::resetChecksum() {
    switch (m_version) {
        case 1:
            m_v1BufferTotalLength = 0;
            break;
    }
    m_isEncodingChecksum = false;
}

bool ChecksumCalculator::validate(const void* expectedChecksum, size_t expectedChecksumLen) {
    size_t checksumSize = checksumByteSize();
    if (expectedChecksumLen != checksumSize) {
        m_numRead++;
        resetChecksum();
        return false;
    }
    // buffers for computing the checksum
    unsigned char sChecksumBuffer[kMaxChecksumSize];
    switch (m_version) {
        case 1: {
            uint32_t val = computeV1Checksum();
            memcpy(sChecksumBuffer, &val, sizeof(val));
            memcpy(sChecksumBuffer+sizeof(val), &m_numRead, sizeof(m_numRead));
            break;
        }
    }
    bool isValid = !memcmp(sChecksumBuffer, expectedChecksum, checksumSize);
    m_numRead++;
    resetChecksum();
    return isValid;
}

uint32_t ChecksumCalculator::computeV1Checksum() {
    uint32_t revLen = m_v1BufferTotalLength;
    revLen = (revLen & 0xffff0000) >> 16 | (revLen & 0x0000ffff) << 16;
    revLen = (revLen & 0xff00ff00) >> 8 | (revLen & 0x00ff00ff) << 8;
    revLen = (revLen & 0xf0f0f0f0) >> 4 | (revLen & 0x0f0f0f0f) << 4;
    revLen = (revLen & 0xcccccccc) >> 2 | (revLen & 0x33333333) << 2;
    revLen = (revLen & 0xaaaaaaaa) >> 1 | (revLen & 0x55555555) << 1;
    return revLen;
}
