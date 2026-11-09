/* SPDX-FileCopyrightText: 2026 Ephemeral Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include "BLI_math_matrix.hh"

#include "EPH_solver.hh"

#include "testing/testing.h"

namespace blender::ephemeral::tests {

TEST(ephemeral_solver, aim_bone_turns_toward_target)
{
  /* Bone at (1, 0, 0) pointing up +Y, length 2, child head at its tail. */
  float4x4 pose_mat = float4x4::identity();
  pose_mat.location() = float3(1.0f, 0.0f, 0.0f);
  const float3 child_head(1.0f, 2.0f, 0.0f);

  /* Pull the child head toward +X, much further out than the bone is long. */
  const float4x4 aimed = aim_bone(pose_mat, child_head, float3(10.0f, 0.0f, 0.0f));

  EXPECT_V3_NEAR(aimed.location(), float3(1.0f, 0.0f, 0.0f), 1e-6f); /* Head stays. */
  EXPECT_V3_NEAR(aimed.y_axis(), float3(1.0f, 0.0f, 0.0f), 1e-6f);    /* Now points +X. */
  EXPECT_V3_NEAR(aimed.z_axis(), float3(0.0f, 0.0f, 1.0f), 1e-6f);    /* No twist. */
  /* Length locked: the attached point lands 2 units out, not at the target. */
  const float3 new_child_head = math::transform_point(aimed * math::invert(pose_mat),
                                                      child_head);
  EXPECT_V3_NEAR(new_child_head, float3(3.0f, 0.0f, 0.0f), 1e-5f);
}

TEST(ephemeral_solver, aim_bone_keeps_pose_when_degenerate)
{
  float4x4 pose_mat = float4x4::identity();
  pose_mat.location() = float3(0.0f, 0.0f, 1.0f);
  const float4x4 aimed = aim_bone(pose_mat, float3(0.0f, 1.0f, 1.0f), float3(0.0f, 0.0f, 1.0f));
  EXPECT_M4_NEAR(aimed.ptr(), pose_mat.ptr(), 1e-6f);
}

}  // namespace blender::ephemeral::tests
