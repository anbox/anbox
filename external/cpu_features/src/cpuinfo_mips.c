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

#include "cpuinfo_mips.h"

#include "internal/filesystem.h"
#include "internal/stack_line_reader.h"
#include "internal/string_view.h"
#include "internal/unix_features_aggregator.h"

#include <assert.h>

DECLARE_SETTER_AND_GETTER(MipsFeatures, msa)
DECLARE_SETTER_AND_GETTER(MipsFeatures, eva)
DECLARE_SETTER_AND_GETTER(MipsFeatures, r6)

static const CapabilityConfig kConfigs[] = {
  [MIPS_MSA] = {{MIPS_HWCAP_MSA, 0}, "msa", &set_msa, &get_msa},  //
  [MIPS_EVA] = {{0, 0}, "eva", &set_eva, &get_eva},               //
  [MIPS_R6] = {{MIPS_HWCAP_R6, 0}, "r6", &set_r6, &get_r6},       //
};
static const size_t kConfigsSize = sizeof(kConfigs) / sizeof(CapabilityConfig);

static bool HandleMipsLine(const LineResult result,
                           MipsFeatures* const features) {
  StringView key, value;
  // See tests for an example.
  if (CpuFeatures_StringView_GetAttributeKeyValue(result.line, &key, &value)) {
    if (CpuFeatures_StringView_IsEquals(key, str("ASEs implemented"))) {
      CpuFeatures_SetFromFlags(kConfigsSize, kConfigs, value, features);
    }
  }
  return !result.eof;
}

static void FillProcCpuInfoData(MipsFeatures* const features) {
  const int fd = CpuFeatures_OpenFile("/proc/cpuinfo");
  if (fd >= 0) {
    StackLineReader reader;
    StackLineReader_Initialize(&reader, fd);
    for (;;) {
      if (!HandleMipsLine(StackLineReader_NextLine(&reader), features)) {
        break;
      }
    }
    CpuFeatures_CloseFile(fd);
  }
}

static const MipsInfo kEmptyMipsInfo;

MipsInfo GetMipsInfo(void) {
  assert(kConfigsSize == MIPS_LAST_);

  // capabilities are fetched from both getauxval and /proc/cpuinfo so we can
  // have some information if the executable is sandboxed (aka no access to
  // /proc/cpuinfo).
  MipsInfo info = kEmptyMipsInfo;

  FillProcCpuInfoData(&info.features);
  CpuFeatures_OverrideFromHwCaps(kConfigsSize, kConfigs,
                                 CpuFeatures_GetHardwareCapabilities(),
                                 &info.features);
  return info;
}

////////////////////////////////////////////////////////////////////////////////
// Introspection functions

int GetMipsFeaturesEnumValue(const MipsFeatures* features,
                             MipsFeaturesEnum value) {
  if(value >= kConfigsSize)
    return false;
  return kConfigs[value].get_bit((MipsFeatures*)features);
}

const char* GetMipsFeaturesEnumName(MipsFeaturesEnum value) {
  if(value >= kConfigsSize)
    return "unknown feature";
  return kConfigs[value].proc_cpuinfo_flag;
}
