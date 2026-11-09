/* SPDX-FileCopyrightText: 2026 Ephemeral Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup ephemeral
 */

#include "BLI_math_matrix.h"
#include "BLI_math_rotation.h"
#include "BLI_math_vector.hh"

#include "EPH_solver.hh"

namespace blender::ephemeral {

float4x4 aim_bone(const float4x4 &pose_mat, const float3 &from, const float3 &to)
{
  const float3 head = pose_mat.location();
  const float3 dir_from = from - head;
  const float3 dir_to = to - head;
  if (math::is_zero(dir_from) || math::is_zero(dir_to)) {
    return pose_mat;
  }

  float rotation[3][3];
  rotation_between_vecs_to_mat3(rotation, math::normalize(dir_from), math::normalize(dir_to));
  float rotscale[3][3];
  copy_m3_m4(rotscale, pose_mat.ptr());
  float aimed[3][3];
  mul_m3_m3m3(aimed, rotation, rotscale);

  float4x4 result;
  copy_m4_m3(result.ptr(), aimed);
  result.location() = head;
  return result;
}

}  // namespace blender::ephemeral
