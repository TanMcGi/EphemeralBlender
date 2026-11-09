/* SPDX-FileCopyrightText: 2026 Ephemeral Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup DNA
 *
 * Pose Sequence: interpolationless animation stored as explicit poses.
 */

#pragma once

#include "DNA_ID.h"

namespace blender {

/** One bone's transform in one stored pose. */
struct PoseSequenceTransform {
  float location[3] = {0.0f, 0.0f, 0.0f};
  /** Quaternion (w, x, y, z). */
  float rotation[4] = {1.0f, 0.0f, 0.0f, 0.0f};
  float scale[3] = {1.0f, 1.0f, 1.0f};
};

struct PoseSequenceBone {
  char name[/*MAXBONENAME*/ 64] = "";
};

/** #PoseSequence.frame_states */
enum ePoseSequenceFrameState : uint8_t {
  /** Nothing stored; the frame shows the nearest earlier stored frame. */
  POSE_SEQUENCE_FRAME_EMPTY = 0,
  /** Posed by the animator. */
  POSE_SEQUENCE_FRAME_AUTHORED = 1,
  /** Filled in by a tool. */
  POSE_SEQUENCE_FRAME_GENERATED = 2,
};

struct PoseSequence {
#ifdef __cplusplus
  /** See #ID_Type comment for why this is here. */
  static constexpr ID_Type id_type = ID_PS;
#endif

  ID id;

  /** Scene frame stored at index 0. */
  int frame_start = 1;
  int frame_count = 0;
  int bone_count = 0;
  char _pad[4] = {};

  /** Bones the poses cover, matched to an armature by name. Size #bone_count. */
  PoseSequenceBone *bones = nullptr;
  /** One #ePoseSequenceFrameState per frame. Size #frame_count. */
  uint8_t *frame_states = nullptr;
  /** Frame-major: frame `f`, bone `b` is at `f * bone_count + b`. Size `frame_count * bone_count`. */
  PoseSequenceTransform *transforms = nullptr;
};

}  // namespace blender
