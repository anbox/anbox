// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "cpuinfo_aarch64.h"
#include "filesystem_for_testing.h"
#include "hwcaps_for_testing.h"

#include "gtest/gtest.h"

namespace cpu_features {
namespace {

void DisableHardwareCapabilities() { SetHardwareCapabilities(0, 0); }

TEST(CpuinfoAarch64Test, FromHardwareCap) {
  SetHardwareCapabilities(AARCH64_HWCAP_FP | AARCH64_HWCAP_AES, 0);
  GetEmptyFilesystem();  // disabling /proc/cpuinfo
  const auto info = GetAarch64Info();
  EXPECT_TRUE(info.features.fp);
  EXPECT_FALSE(info.features.asimd);
  EXPECT_FALSE(info.features.evtstrm);
  EXPECT_TRUE(info.features.aes);
  EXPECT_FALSE(info.features.pmull);
  EXPECT_FALSE(info.features.sha1);
  EXPECT_FALSE(info.features.sha2);
  EXPECT_FALSE(info.features.crc32);
  EXPECT_FALSE(info.features.atomics);
  EXPECT_FALSE(info.features.fphp);
  EXPECT_FALSE(info.features.asimdhp);
  EXPECT_FALSE(info.features.cpuid);
  EXPECT_FALSE(info.features.asimdrdm);
  EXPECT_FALSE(info.features.jscvt);
  EXPECT_FALSE(info.features.fcma);
  EXPECT_FALSE(info.features.lrcpc);
  EXPECT_FALSE(info.features.dcpop);
  EXPECT_FALSE(info.features.sha3);
  EXPECT_FALSE(info.features.sm3);
  EXPECT_FALSE(info.features.sm4);
  EXPECT_FALSE(info.features.asimddp);
  EXPECT_FALSE(info.features.sha512);
  EXPECT_FALSE(info.features.sve);
  EXPECT_FALSE(info.features.asimdfhm);
  EXPECT_FALSE(info.features.dit);
  EXPECT_FALSE(info.features.uscat);
  EXPECT_FALSE(info.features.ilrcpc);
  EXPECT_FALSE(info.features.flagm);
  EXPECT_FALSE(info.features.ssbs);
  EXPECT_FALSE(info.features.sb);
  EXPECT_FALSE(info.features.paca);
  EXPECT_FALSE(info.features.pacg);
}

TEST(CpuinfoAarch64Test, ARMCortexA53) {
  DisableHardwareCapabilities();
  auto& fs = GetEmptyFilesystem();
  fs.CreateFile("/proc/cpuinfo",
                R"(Processor   : AArch64 Processor rev 3 (aarch64)
processor   : 0
processor   : 1
processor   : 2
processor   : 3
processor   : 4
processor   : 5
processor   : 6
processor   : 7
Features    : fp asimd evtstrm aes pmull sha1 sha2 crc32
CPU implementer : 0x41
CPU architecture: AArch64
CPU variant : 0x0
CPU part    : 0xd03
CPU revision    : 3)");
  const auto info = GetAarch64Info();
  EXPECT_EQ(info.implementer, 0x41);
  EXPECT_EQ(info.variant, 0x0);
  EXPECT_EQ(info.part, 0xd03);
  EXPECT_EQ(info.revision, 3);

  EXPECT_TRUE(info.features.fp);
  EXPECT_TRUE(info.features.asimd);
  EXPECT_TRUE(info.features.evtstrm);
  EXPECT_TRUE(info.features.aes);
  EXPECT_TRUE(info.features.pmull);
  EXPECT_TRUE(info.features.sha1);
  EXPECT_TRUE(info.features.sha2);
  EXPECT_TRUE(info.features.crc32);

  EXPECT_FALSE(info.features.atomics);
  EXPECT_FALSE(info.features.fphp);
  EXPECT_FALSE(info.features.asimdhp);
  EXPECT_FALSE(info.features.cpuid);
  EXPECT_FALSE(info.features.asimdrdm);
  EXPECT_FALSE(info.features.jscvt);
  EXPECT_FALSE(info.features.fcma);
  EXPECT_FALSE(info.features.lrcpc);
  EXPECT_FALSE(info.features.dcpop);
  EXPECT_FALSE(info.features.sha3);
  EXPECT_FALSE(info.features.sm3);
  EXPECT_FALSE(info.features.sm4);
  EXPECT_FALSE(info.features.asimddp);
  EXPECT_FALSE(info.features.sha512);
  EXPECT_FALSE(info.features.sve);
  EXPECT_FALSE(info.features.asimdfhm);
  EXPECT_FALSE(info.features.dit);
  EXPECT_FALSE(info.features.uscat);
  EXPECT_FALSE(info.features.ilrcpc);
  EXPECT_FALSE(info.features.flagm);
  EXPECT_FALSE(info.features.ssbs);
  EXPECT_FALSE(info.features.sb);
  EXPECT_FALSE(info.features.paca);
  EXPECT_FALSE(info.features.pacg);
}

}  // namespace
}  // namespace cpu_features
