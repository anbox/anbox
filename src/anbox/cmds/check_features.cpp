/*
 * Copyright (C) 2018 Simon Fels <morphis@gravedo.de>
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

#include "anbox/cmds/check_features.h"
#include "anbox/utils.h"

#include "cpu_features_macros.h"
#include "cpuinfo_x86.h"

namespace {
std::vector<std::string> cpu_whitelist = {
  // QEMU does not necessarily expose correctly that it supports SSE and friends even
  // when started with `-cpu qemu64,+ssse3,+sse4.1,+sse4.2,+x2apic`
  "QEMU",

  // The following CPUs do not support AVX and without it cpu_features can't detect
  // if SSE & friends are supported. See https://github.com/google/cpu_features/issues/4

  // Intel Core i7 M620
  "M 620",
  // Intel Core i5 M460
  "M 460",
  // Intel Celeron N2840
  "N2840",
  // Intel Core i7 Q720
  "Q 720",
  // Intel Pentium T4500
  "T4500", 
  // Intel Core i7 Q720
  "Q 720",
  // Intel Xeon E5520
  "E5520"
  // Intel Core2 Duo T6500
  "T6500"
};
} // namespace

anbox::cmds::CheckFeatures::CheckFeatures()
    : CommandWithFlagsAndAction{
          cli::Name{"check-features"}, cli::Usage{"check-features"},
          cli::Description{"Check that the host system supports all necessary features"}} {

  action([this](const cli::Command::Context&) {
#if defined(CPU_FEATURES_ARCH_X86)
    const auto info = cpu_features::GetX86Info();
    std::vector<std::string> missing_features;

#define CHECK_BOOL(x, name) \
  if (!x) \
    missing_features.push_back(name)

    CHECK_BOOL(info.features.sse4_1, "SSE 4.1");
    CHECK_BOOL(info.features.sse4_2, "SSE 4.2");
    CHECK_BOOL(info.features.ssse3, "SSSE 3");

    char brand_string[49];
    cpu_features::FillX86BrandString(brand_string);
    std::string brand(brand_string);

    // Check if we have a CPU which's features we can't detect correctly
    auto is_whitelisted = false;
    for (const auto &entry : cpu_whitelist) {
      if (brand.find(entry) != std::string::npos) {
        is_whitelisted = true;
        break;
      }
    }

    if (missing_features.size() > 0 && !is_whitelisted && !sanity_check_for_features()) {
      std::cerr << "The CPU of your computer (" << brand_string << ") does not support all" << std::endl
                << "features Anbox requires." << std::endl
                << "It is missing support for the following features: ";

      for (size_t n = 0; n < missing_features.size(); n++) {
        const auto feature = missing_features[n];
        std::cerr << feature;
        if (n < missing_features.size() - 1)
          std::cerr << ", ";
      }
      std::cerr << std::endl;
      std::cerr << "You can for example find more information about SSE" << std::endl
                << "here https://en.wikipedia.org/wiki/Streaming_SIMD_Extensions" << std::endl;

      return EXIT_FAILURE;
    }

    std::cout << "Your computer does meet all requirements to run Anbox" << std::endl;

    return EXIT_SUCCESS;
#else
    std::cerr << "You're running Anbox on a not yet supported architecture" << std::endl;
    return EXIT_FAILURE;
#endif
  });
}

// In case that the CPU supports AVX we take the decision as from our analysis
// of the output from the cpu_features library. If it does not we have to check
// further via the compiler builtins if we the CPU supports all mandatory features
// or not. In case that any is missing we will fail the test.
//
// This uses the compiler builtin function __builtin_cpu_supports which allows us
// to detect certain CPU features.
// See https://gcc.gnu.org/onlinedocs/gcc/x86-Built-in-Functions.html
bool anbox::cmds::CheckFeatures::sanity_check_for_features() {
#if defined(CPU_FEATURES_ARCH_X86)
  if (__builtin_cpu_supports("avx"))
    return true;

  std::vector<std::string> missing_features;

#define CHECK_FEATURE(name) \
  if (!__builtin_cpu_supports(name)) \
    missing_features.push_back(name);

  CHECK_FEATURE("sse4.1");
  CHECK_FEATURE("sse4.2");
  CHECK_FEATURE("ssse3");

  return missing_features.empty();
#else
  return true;
#endif
}
