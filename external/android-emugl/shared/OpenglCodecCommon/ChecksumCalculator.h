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

#include <stdint.h>
#include <stdlib.h>

// Set TRACE_CHECKSUMHELPER to 1 to debug creation/destruction of GLprotocol
// instances.
#define TRACE_CHECKSUMHELPER 0

#if TRACE_CHECKSUMHELPER
#define LOG_CHECKSUMHELPER(x...) fprintf(stderr, x)
#else
#define LOG_CHECKSUMHELPER(x...)
#endif

// ChecksumCalculator adds checksum as an array of bytes to GL pipe communication, which
// size depends on the protocol version. Each pipe should use one ChecksumCalculator.
// It can:
//      (1) take a list of buffers one by one and compute their checksum string,
//          in this case the checksum should be as the data in those buffers are
//          concatenated;
//      (2) compute the checksum of the buffer list, then either write them into
//          a buffer provided by user, or compare it against a checksum provided
//          by user
//      (3) support different checksum version in future.
//
// For backward compatibility, checksum version 0 behaves the same as there is
// no checksum (i.e., checksumByteSize returns 0, validate always returns true,
// addBuffer and writeCheckSum does nothing).
//
// Notice that to detect package lost, ChecksumCalculator also keeps track of how
// many times it generates/validates checksums, and might use it as part of the
// checksum.
//
// To evaluate checksums from a list of data buffers buf1, buf2... Please call
// addBuffer(buf1, buf1len), addBuffer(buf2, buf2len) ... in order.
// Then if the checksum needs to be encoded into a buffer, one needs to allocate
// a checksum buffer with size checksumByteSize(), and call
// writeChecksum(checksumBuffer) to write the checksum to the buffer.
// If the checksum needs to be validated against an existing one, one needs to
// call validate(existChecksum, existChecksumLen).
//
// The checksum generator and validator must be set to the same version, and
// the validator must check ALL checksums in the order they are generated,
// otherwise the validation function will return false.
//
// It is allowed to change the checksum version between calculating two
// checksums. This is designed for backward compatibility reason.
//
// Example 1, encoding and decoding:
//
// bool testChecksum(void* buf, size_t bufLen) {
//     // encoding message
//     ChecksumCalculator encoder;
//     encoder.setVersion(1);
//     encoder.addBuffer(buf, bufLen);
//     std::vector<unsigned char> message(bufLen + encoder.checksumByteSize());
//     memcpy(&message[0], buf, bufLen);
//     encoder.writeChecksum(&message[0] + bufLen, encoder.checksumByteSize());
//
//     // decoding message
//     ChecksumCalculator decoder;
//     decoder.setVersion(1);
//     decoder.addBuffer(&message[0], bufLen);
//     return decoder.validate(&message[0] + bufLen, decoder.checksumByteSize());
// }
// The return value is true.
//
// Example 2, decoding will fail if the order of messages is wrong:
//
// bool testChecksumOrder(void* buf1, size_t bufLen1,
//                        void* buf2, size_t bufLen2) {
//     // encoding messages
//     ChecksumCalculator encoder;
//     encoder.setVersion(1);
//
//     std::vector<unsigned char> message1(bufLen1 + encoder.checksumByteSize());
//     std::vector<unsigned char> message2(bufLen2 + encoder.checksumByteSize());
//
//     encoder.addBuffer(buf1, bufLen1);
//     std::vector<unsigned char> message1(bufLen1 + encoder.checksumByteSize());
//     memcpy(&message1[0], buf1, bufLen1);
//     encoder.writeChecksum(&message1[0] + bufLen1, encoder.checksumByteSize());
//
//     encoder.addBuffer(buf2, bufLen2);
//     std::vector<unsigned char> message2(bufLen2 + encoder.checksumByteSize());
//     memcpy(&message2[0], buf2, bufLen2);
//     encoder.writeChecksum(&message2[0] + bufLen2, encoder.checksumByteSize());
//
//     // decoding messages
//     ChecksumCalculator decoder;
//     decoder.setVersion(1);
//     decoder.addBuffer(&message2[0], bufLen2);
//     // returns false because the decoding order is not consistent with
//     // encoding order
//     if (!decoder.validate(&message2[0]+bufLen2, decoder.checksumByteSize())) {
//         return false;
//     }
//
//     decoder.addBuffer(&message1[0], bufLen1);
//     if (!decoder.validate(&message1[0]+bufLen1, decoder.checksumByteSize())) {
//         return false;
//     }
//
//     return false;
// }

class ChecksumCalculator {
public:
    // Get and set current checksum version
    uint32_t getVersion() const { return m_version; }
    // Call setVersion to set a checksum version. It should be called before
    // addBuffer(), writeChecksum() and validate(). And it should be called
    // exact once per rendering thread if both host and guest support checksum.
    // It won't be called if either host or guest does not support checksum.
    bool setVersion(uint32_t version);

    // Maximum supported checksum version
    static uint32_t getMaxVersion();
    // A version string that looks like "ANDROID_EMU_CHECKSUM_HELPER_v1"
    // Used multiple times when the guest queries the maximum supported version
    // from the host.
    // The library owns the returned pointer. The returned pointer will be
    // deconstructed when unloading library.
    static const char* getMaxVersionStr();
    static const char* getMaxVersionStrPrefix();

    // Size of checksum in the current version
    size_t checksumByteSize() const;

    // Update the current checksum value from the data
    // at |buf| of |bufLen| bytes. Once all buffers
    // have been added, call writeChecksum() to store
    // the final checksum value and reset its state.
    void addBuffer(const void* buf, size_t bufLen);
    // Write the checksum from the list of buffers to outputChecksum
    // Will reset the list of buffers by calling resetChecksum.
    // Return false if the buffer is not long enough
    // Please query buffer size from checksumByteSize()
    bool writeChecksum(void* outputChecksum, size_t outputChecksumLen);
    // Restore the states for computing checksums.
    // Automatically called at the end of writeChecksum and validate.
    // Can also be used to abandon the current checksum being calculated.
    // Notes: it doesn't update the internal read / write counter
    void resetChecksum();

    // Calculate the checksum from the list of buffers and
    // compare it with the checksum encoded in expectedChecksum
    // Will reset the list of buffers by calling resetChecksum.
    bool validate(const void* expectedChecksum, size_t expectedChecksumLen);
protected:
    uint32_t m_version = 0;
    // A temporary state used to compute the total length of a list of buffers,
    // if addBuffer is called.
    uint32_t m_numRead = 0;
    uint32_t m_numWrite = 0;
    // m_isEncodingChecksum is true when between addBuffer and writeChecksum
    bool m_isEncodingChecksum = false;
private:
    // Compute a 32bit checksum
    // Used in protocol v1
    uint32_t computeV1Checksum();
    // The buffer used in protocol version 1 to compute checksum.
    uint32_t m_v1BufferTotalLength = 0;
};
