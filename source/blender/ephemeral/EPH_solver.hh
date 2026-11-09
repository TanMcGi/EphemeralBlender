/* SPDX-FileCopyrightText: 2026 Ephemeral Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

/** \file
 * \ingroup ephemeral
 */

#include "BLI_math_matrix_types.hh"
#include "BLI_math_vector_types.hh"

namespace blender::ephemeral {

/**
 * Aim: rotate a bone about its head by the smallest rotation that turns the point `from` toward
 * `to`, as if `from` were rigidly attached to the bone. Lengths are preserved.
 *
 * \param pose_mat: The bone's pose matrix (armature space), head at its location.
 * \param from: A point attached to the bone, such as a child's head (armature space).
 * \param to: Where that point should point (armature space).
 * \return The aimed pose matrix. Unchanged when either point is at the head.
 */
float4x4 aim_bone(const float4x4 &pose_mat, const float3 &from, const float3 &to);

}  // namespace blender::ephemeral
