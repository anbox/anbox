// Copyright 2018 IBM.
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

#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "cpuinfo_ppc.h"
#include "internal/bit_utils.h"
#include "internal/filesystem.h"
#include "internal/stack_line_reader.h"
#include "internal/string_view.h"
#include "internal/unix_features_aggregator.h"

DECLARE_SETTER_AND_GETTER(PPCFeatures, ppc32)
DECLARE_SETTER_AND_GETTER(PPCFeatures, ppc64)
DECLARE_SETTER_AND_GETTER(PPCFeatures, ppc601)
DECLARE_SETTER_AND_GETTER(PPCFeatures, altivec)
DECLARE_SETTER_AND_GETTER(PPCFeatures, fpu)
DECLARE_SETTER_AND_GETTER(PPCFeatures, mmu)
DECLARE_SETTER_AND_GETTER(PPCFeatures, mac_4xx)
DECLARE_SETTER_AND_GETTER(PPCFeatures, unifiedcache)
DECLARE_SETTER_AND_GETTER(PPCFeatures, spe)
DECLARE_SETTER_AND_GETTER(PPCFeatures, efpsingle)
DECLARE_SETTER_AND_GETTER(PPCFeatures, efpdouble)
DECLARE_SETTER_AND_GETTER(PPCFeatures, no_tb)
DECLARE_SETTER_AND_GETTER(PPCFeatures, power4)
DECLARE_SETTER_AND_GETTER(PPCFeatures, power5)
DECLARE_SETTER_AND_GETTER(PPCFeatures, power5plus)
DECLARE_SETTER_AND_GETTER(PPCFeatures, cell)
DECLARE_SETTER_AND_GETTER(PPCFeatures, booke)
DECLARE_SETTER_AND_GETTER(PPCFeatures, smt)
DECLARE_SETTER_AND_GETTER(PPCFeatures, icachesnoop)
DECLARE_SETTER_AND_GETTER(PPCFeatures, arch205)
DECLARE_SETTER_AND_GETTER(PPCFeatures, pa6t)
DECLARE_SETTER_AND_GETTER(PPCFeatures, dfp)
DECLARE_SETTER_AND_GETTER(PPCFeatures, power6ext)
DECLARE_SETTER_AND_GETTER(PPCFeatures, arch206)
DECLARE_SETTER_AND_GETTER(PPCFeatures, vsx)
DECLARE_SETTER_AND_GETTER(PPCFeatures, pseries_perfmon_compat)
DECLARE_SETTER_AND_GETTER(PPCFeatures, truele)
DECLARE_SETTER_AND_GETTER(PPCFeatures, ppcle)
DECLARE_SETTER_AND_GETTER(PPCFeatures, arch207)
DECLARE_SETTER_AND_GETTER(PPCFeatures, htm)
DECLARE_SETTER_AND_GETTER(PPCFeatures, dscr)
DECLARE_SETTER_AND_GETTER(PPCFeatures, ebb)
DECLARE_SETTER_AND_GETTER(PPCFeatures, isel)
DECLARE_SETTER_AND_GETTER(PPCFeatures, tar)
DECLARE_SETTER_AND_GETTER(PPCFeatures, vcrypto)
DECLARE_SETTER_AND_GETTER(PPCFeatures, htm_nosc)
DECLARE_SETTER_AND_GETTER(PPCFeatures, arch300)
DECLARE_SETTER_AND_GETTER(PPCFeatures, ieee128)
DECLARE_SETTER_AND_GETTER(PPCFeatures, darn)
DECLARE_SETTER_AND_GETTER(PPCFeatures, scv)
DECLARE_SETTER_AND_GETTER(PPCFeatures, htm_no_suspend)

static const CapabilityConfig kConfigs[] = {
  [PPC_32] = {{PPC_FEATURE_32, 0}, "ppc32", &set_ppc32, &get_ppc32},
  [PPC_64] = {{PPC_FEATURE_64, 0}, "ppc64", &set_ppc64, &get_ppc64},
  [PPC_601_INSTR] = {{PPC_FEATURE_601_INSTR, 0}, "ppc601", &set_ppc601, &get_ppc601},
  [PPC_HAS_ALTIVEC] = {{PPC_FEATURE_HAS_ALTIVEC, 0}, "altivec", &set_altivec, &get_altivec},
  [PPC_HAS_FPU] = {{PPC_FEATURE_HAS_FPU, 0}, "fpu", &set_fpu, &get_fpu},
  [PPC_HAS_MMU] = {{PPC_FEATURE_HAS_MMU, 0}, "mmu", &set_mmu, &get_mmu},
  [PPC_HAS_4xxMAC] = {{PPC_FEATURE_HAS_4xxMAC, 0}, "4xxmac", &set_mac_4xx, &get_mac_4xx},
  [PPC_UNIFIED_CACHE] = {{PPC_FEATURE_UNIFIED_CACHE, 0}, "ucache", &set_unifiedcache, &get_unifiedcache},
  [PPC_HAS_SPE] = {{PPC_FEATURE_HAS_SPE, 0}, "spe", &set_spe, &get_spe},
  [PPC_HAS_EFP_SINGLE] = {{PPC_FEATURE_HAS_EFP_SINGLE, 0}, "efpsingle", &set_efpsingle, &get_efpsingle},
  [PPC_HAS_EFP_DOUBLE] = {{PPC_FEATURE_HAS_EFP_DOUBLE, 0}, "efpdouble", &set_efpdouble, &get_efpdouble},
  [PPC_NO_TB] = {{PPC_FEATURE_NO_TB, 0}, "notb", &set_no_tb, &get_no_tb},
  [PPC_POWER4] = {{PPC_FEATURE_POWER4, 0}, "power4", &set_power4, &get_power4},
  [PPC_POWER5] = {{PPC_FEATURE_POWER5, 0}, "power5", &set_power5, &get_power5},
  [PPC_POWER5_PLUS] = {{PPC_FEATURE_POWER5_PLUS, 0}, "power5+", &set_power5plus, &get_power5plus},
  [PPC_CELL] = {{PPC_FEATURE_CELL, 0}, "cellbe", &set_cell, &get_cell},
  [PPC_BOOKE] = {{PPC_FEATURE_BOOKE, 0}, "booke", &set_booke, &get_booke},
  [PPC_SMT] = {{PPC_FEATURE_SMT, 0}, "smt", &set_smt, &get_smt},
  [PPC_ICACHE_SNOOP] = {{PPC_FEATURE_ICACHE_SNOOP, 0}, "ic_snoop", &set_icachesnoop, &get_icachesnoop},
  [PPC_ARCH_2_05] = {{PPC_FEATURE_ARCH_2_05, 0}, "arch_2_05", &set_arch205, &get_arch205},
  [PPC_PA6T] = {{PPC_FEATURE_PA6T, 0}, "pa6t", &set_pa6t, &get_pa6t},
  [PPC_HAS_DFP] = {{PPC_FEATURE_HAS_DFP, 0}, "dfp", &set_dfp, &get_dfp},
  [PPC_POWER6_EXT] = {{PPC_FEATURE_POWER6_EXT, 0}, "power6x", &set_power6ext, &get_power6ext},
  [PPC_ARCH_2_06] = {{PPC_FEATURE_ARCH_2_06, 0}, "arch_2_06", &set_arch206, &get_arch206},
  [PPC_HAS_VSX] = {{PPC_FEATURE_HAS_VSX, 0}, "vsx", &set_vsx, &get_vsx},
  [PPC_PSERIES_PERFMON_COMPAT] = {{PPC_FEATURE_PSERIES_PERFMON_COMPAT, 0}, "archpmu",
     &set_pseries_perfmon_compat, &get_pseries_perfmon_compat},
  [PPC_TRUE_LE] = {{PPC_FEATURE_TRUE_LE, 0}, "true_le", &set_truele, &get_truele},
  [PPC_PPC_LE] = {{PPC_FEATURE_PPC_LE, 0}, "ppcle", &set_ppcle, &get_ppcle},
  [PPC_ARCH_2_07] = {{0, PPC_FEATURE2_ARCH_2_07}, "arch_2_07", &set_arch207, &get_arch207},
  [PPC_HTM] = {{0, PPC_FEATURE2_HTM}, "htm", &set_htm, &get_htm},
  [PPC_DSCR] = {{0, PPC_FEATURE2_DSCR}, "dscr", &set_dscr, &get_dscr},
  [PPC_EBB] = {{0, PPC_FEATURE2_EBB}, "ebb", &set_ebb, &get_ebb},
  [PPC_ISEL] = {{0, PPC_FEATURE2_ISEL}, "isel", &set_isel, &get_isel},
  [PPC_TAR] = {{0, PPC_FEATURE2_TAR}, "tar", &set_tar, &get_tar},
  [PPC_VEC_CRYPTO] = {{0, PPC_FEATURE2_VEC_CRYPTO}, "vcrypto", &set_vcrypto, &get_vcrypto},
  [PPC_HTM_NOSC] = {{0, PPC_FEATURE2_HTM_NOSC}, "htm-nosc", &set_htm_nosc, &get_htm_nosc},
  [PPC_ARCH_3_00] = {{0, PPC_FEATURE2_ARCH_3_00}, "arch_3_00", &set_arch300, &get_arch300},
  [PPC_HAS_IEEE128] = {{0, PPC_FEATURE2_HAS_IEEE128}, "ieee128", &set_ieee128, &get_ieee128},
  [PPC_DARN] = {{0, PPC_FEATURE2_DARN}, "darn", &set_darn, &get_darn},
  [PPC_SCV] = {{0, PPC_FEATURE2_SCV}, "scv", &set_scv, &get_scv},
  [PPC_HTM_NO_SUSPEND] = {{0, PPC_FEATURE2_HTM_NO_SUSPEND}, "htm-no-suspend", &set_htm_no_suspend,
     &get_htm_no_suspend},
};
static const size_t kConfigsSize = sizeof(kConfigs) / sizeof(CapabilityConfig);

static bool HandlePPCLine(const LineResult result,
                          PPCPlatformStrings* const strings) {
  StringView line = result.line;
  StringView key, value;
  if (CpuFeatures_StringView_GetAttributeKeyValue(line, &key, &value)) {
    if (CpuFeatures_StringView_HasWord(key, "platform")) {
      CpuFeatures_StringView_CopyString(value, strings->platform,
                                        sizeof(strings->platform));
    } else if (CpuFeatures_StringView_IsEquals(key, str("model"))) {
      CpuFeatures_StringView_CopyString(value, strings->model,
                                        sizeof(strings->platform));
    } else if (CpuFeatures_StringView_IsEquals(key, str("machine"))) {
      CpuFeatures_StringView_CopyString(value, strings->machine,
                                        sizeof(strings->platform));
    } else if (CpuFeatures_StringView_IsEquals(key, str("cpu"))) {
      CpuFeatures_StringView_CopyString(value, strings->cpu,
                                        sizeof(strings->platform));
    }
  }
  return !result.eof;
}

static void FillProcCpuInfoData(PPCPlatformStrings* const strings) {
  const int fd = CpuFeatures_OpenFile("/proc/cpuinfo");
  if (fd >= 0) {
    StackLineReader reader;
    StackLineReader_Initialize(&reader, fd);
    for (;;) {
      if (!HandlePPCLine(StackLineReader_NextLine(&reader), strings)) {
        break;
      }
    }
    CpuFeatures_CloseFile(fd);
  }
}

static const PPCInfo kEmptyPPCInfo;

PPCInfo GetPPCInfo(void) {
  /*
   * On Power feature flags aren't currently in cpuinfo so we only look at
   * the auxilary vector.
   */
  PPCInfo info = kEmptyPPCInfo;

  CpuFeatures_OverrideFromHwCaps(kConfigsSize, kConfigs,
                                 CpuFeatures_GetHardwareCapabilities(),
                                 &info.features);
  return info;
}

static const PPCPlatformStrings kEmptyPPCPlatformStrings;

PPCPlatformStrings GetPPCPlatformStrings(void) {
  PPCPlatformStrings strings = kEmptyPPCPlatformStrings;

  FillProcCpuInfoData(&strings);
  strings.type = CpuFeatures_GetPlatformType();
  return strings;
}

////////////////////////////////////////////////////////////////////////////////
// Introspection functions

int GetPPCFeaturesEnumValue(const PPCFeatures* features,
                            PPCFeaturesEnum value) {
  if(value >= kConfigsSize)
    return false;
  return kConfigs[value].get_bit((PPCFeatures*)features);
}

const char* GetPPCFeaturesEnumName(PPCFeaturesEnum value) {
  if(value >= kConfigsSize)
    return "unknown feature";
  return kConfigs[value].proc_cpuinfo_flag;
}
