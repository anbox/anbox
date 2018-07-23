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

#include "cpu_features_macros.h"
#include "cpuinfo_x86.h"

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

    if (missing_features.size() > 0) {
      char brand_string[49];
      cpu_features::FillX86BrandString(brand_string);
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
