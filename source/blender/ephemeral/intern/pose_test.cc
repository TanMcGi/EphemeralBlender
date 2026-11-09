/* SPDX-FileCopyrightText: 2026 Ephemeral Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include <cmath>

#include "BLI_math_constants.h"

#include "EPH_pose.hh"

#include "testing/testing.h"

namespace blender::ephemeral::tests {

TEST(ephemeral_pose, interpolate_halfway)
{
  /* 90 degrees around Z. */
  const float half_angle = float(M_PI_4);
  BoneTransform b;
  b.location = float3(2.0f, 0.0f, 0.0f);
  b.rotation = math::Quaternion(std::cos(half_angle), 0.0f, 0.0f, std::sin(half_angle));
  b.scale = float3(3.0f);

  const BoneTransform result = interpolate(BoneTransform(), b, 0.5f);

  EXPECT_V3_NEAR(result.location, float3(1.0f, 0.0f, 0.0f), 1e-6f);
  EXPECT_V3_NEAR(result.scale, float3(2.0f), 1e-6f);
  /* Halfway is 45 degrees around Z. */
  const float quarter_angle = float(M_PI_4 / 2.0);
  EXPECT_NEAR(result.rotation.w, std::cos(quarter_angle), 1e-6f);
  EXPECT_NEAR(result.rotation.x, 0.0f, 1e-6f);
  EXPECT_NEAR(result.rotation.y, 0.0f, 1e-6f);
  EXPECT_NEAR(result.rotation.z, std::sin(quarter_angle), 1e-6f);
}

}  // namespace blender::ephemeral::tests
