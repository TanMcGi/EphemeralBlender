/* SPDX-FileCopyrightText: 2026 Ephemeral Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

/** \file
 * \ingroup ephemeral
 */

#include "BLI_math_quaternion_types.hh"
#include "BLI_math_vector_types.hh"

namespace blender::ephemeral {

/** One bone's transform within a stored pose. Rotation is always a quaternion. */
struct BoneTransform {
  float3 location = float3(0.0f);
  math::Quaternion rotation = math::Quaternion::identity();
  float3 scale = float3(1.0f);
};

/**
 * Blend between two transforms of the same bone: linear for location and scale,
 * shortest-path spherical for rotation. `t` = 0 gives `a`, `t` = 1 gives `b`.
 */
BoneTransform interpolate(const BoneTransform &a, const BoneTransform &b, float t);

}  // namespace blender::ephemeral
