/* SPDX-FileCopyrightText: 2026 Ephemeral Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

/** \file
 * \ingroup ephemeral
 */

#include "BLI_string_ref.hh"

namespace blender {
struct bArmature;
struct bPose;
struct PoseSequence;
struct PoseSequenceTransform;
}  // namespace blender

namespace blender::ephemeral {

/**
 * Size `seq` for every bone of `armature` over `frame_count` frames starting at scene frame
 * `frame_start`. All frames start empty with rest transforms; any existing poses are discarded.
 */
void pose_sequence_init(PoseSequence &seq,
                        const bArmature &armature,
                        int frame_start,
                        int frame_count);

/**
 * Index of the stored frame shown at scene `frame`: the frame itself when it holds a pose,
 * otherwise the nearest earlier frame that does. Frames past the end show the last stored pose.
 * Returns -1 when nothing is stored at or before `frame`.
 */
int pose_sequence_frame_to_show(const PoseSequence &seq, int frame);

/**
 * Store `transform` for the bone named `bone_name` at scene `frame`, marking the frame authored.
 * An empty frame first takes a copy of the pose it was showing, so other bones keep their pose.
 * Returns false when the frame is outside the sequence or the bone isn't in it.
 */
bool pose_sequence_store_bone(PoseSequence &seq,
                              int frame,
                              StringRefNull bone_name,
                              const PoseSequenceTransform &transform);

/**
 * Store the transforms of every channel in `pose` that the sequence covers at scene `frame`,
 * converting rotations to quaternions, and mark the frame authored.
 * Returns false when the frame is outside the sequence.
 */
bool pose_sequence_store_pose(PoseSequence &seq, int frame, const bPose &pose);

/**
 * Write the pose shown at scene `frame` into the matching channels of `pose`, converting each
 * rotation to the channel's own rotation mode. Channels the sequence doesn't cover are left
 * unchanged, as is everything when no pose is stored at or before `frame`.
 */
void pose_sequence_apply(const PoseSequence &seq, int frame, bPose &pose);

}  // namespace blender::ephemeral
