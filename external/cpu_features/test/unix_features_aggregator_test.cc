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

#include <array>

#include "internal/unix_features_aggregator.h"

#include "gtest/gtest.h"

namespace cpu_features {

namespace {

struct Features {
  bool a = false;
  bool b = false;
  bool c = false;
};

enum eFeatures {
  TEST_a,
  TEST_b,
  TEST_c
};

DECLARE_SETTER_AND_GETTER(Features, a)
DECLARE_SETTER_AND_GETTER(Features, b)
DECLARE_SETTER_AND_GETTER(Features, c)

class LinuxFeatureAggregatorTest : public testing::Test {
 public:
  const std::array<CapabilityConfig, 3> kConfigs = {{
    {{0b0001, 0b0000}, "a", &set_a, &get_a},
    {{0b0010, 0b0000}, "b", &set_b, &get_b},
    {{0b0000, 0b1100}, "c", &set_c, &get_c}
  }};
};

TEST_F(LinuxFeatureAggregatorTest, FromFlagsEmpty) {
  Features features;
  CpuFeatures_SetFromFlags(kConfigs.size(), kConfigs.data(), str(""),
                           &features);
  EXPECT_FALSE(features.a);
  EXPECT_FALSE(features.b);
  EXPECT_FALSE(features.c);

  EXPECT_FALSE(kConfigs[TEST_a].get_bit(&features));
}

TEST_F(LinuxFeatureAggregatorTest, FromFlagsAllSet) {
  Features features;
  CpuFeatures_SetFromFlags(kConfigs.size(), kConfigs.data(), str("a c b"),
                           &features);
  EXPECT_TRUE(features.a);
  EXPECT_TRUE(features.b);
  EXPECT_TRUE(features.c);

  EXPECT_TRUE(kConfigs[TEST_a].get_bit(&features));
}

TEST_F(LinuxFeatureAggregatorTest, FromFlagsOnlyA) {
  Features features;
  CpuFeatures_SetFromFlags(kConfigs.size(), kConfigs.data(), str("a"),
                           &features);
  EXPECT_TRUE(features.a);
  EXPECT_FALSE(features.b);
  EXPECT_FALSE(features.c);

  EXPECT_TRUE(kConfigs[TEST_a].get_bit(&features));
  EXPECT_FALSE(kConfigs[TEST_b].get_bit(&features));
  EXPECT_FALSE(kConfigs[TEST_c].get_bit(&features));
}

TEST_F(LinuxFeatureAggregatorTest, FromHwcapsNone) {
  HardwareCapabilities capability;
  capability.hwcaps = 0;   // matches none
  capability.hwcaps2 = 0;  // matches none
  Features features;
  CpuFeatures_OverrideFromHwCaps(kConfigs.size(), kConfigs.data(), capability,
                                 &features);
  EXPECT_FALSE(features.a);
  EXPECT_FALSE(features.b);
  EXPECT_FALSE(features.c);
}

TEST_F(LinuxFeatureAggregatorTest, FromHwcapsSet) {
  HardwareCapabilities capability;
  capability.hwcaps = 0b0010;   // matches b but not a
  capability.hwcaps2 = 0b1111;  // matches c
  Features features;
  CpuFeatures_OverrideFromHwCaps(kConfigs.size(), kConfigs.data(), capability,
                                 &features);
  EXPECT_FALSE(features.a);
  EXPECT_TRUE(features.b);
  EXPECT_TRUE(features.c);
}

}  // namespace
}  // namespace cpu_features
