/* SPDX-FileCopyrightText: 2026 Ephemeral Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup ephemeral
 */

#include "BLI_math_quaternion.hh"
#include "BLI_math_vector.hh"

#include "EPH_pose.hh"

namespace blender::ephemeral {

BoneTransform interpolate(const BoneTransform &a, const BoneTransform &b, const float t)
{
  BoneTransform result;
  result.location = math::interpolate(a.location, b.location, t);
  result.rotation = math::interpolate(a.rotation, b.rotation, t);
  result.scale = math::interpolate(a.scale, b.scale, t);
  return result;
}

}  // namespace blender::ephemeral
