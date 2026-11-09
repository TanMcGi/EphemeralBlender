/* SPDX-FileCopyrightText: 2026 Ephemeral Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup edtransform
 *
 * Pose-mode transform of armatures that have a Pose Sequence: the edit is written live into the
 * sequence at the current frame. Rotating is plain FK; grabbing a bone that has a parent moves a
 * control at the bone's head and aims the parent at it.
 *
 * Called from the pose conversion in `transform_convert_armature.cc`.
 */

#pragma once

namespace blender {
struct bPoseChannel;
}

namespace blender::ed::transform {

struct TransData;
struct TransDataContainer;
struct TransInfo;

/** Before `tc`'s data is filled: sync its channels with the stored pose and save the frame. */
void pose_sequence_transform_begin(TransInfo *t, TransDataContainer *tc);
/** After `td` is filled for `pchan`: when grabbing a bone with a parent, drive the aim control. */
void pose_sequence_transform_control_init(TransInfo *t,
                                          TransDataContainer *tc,
                                          TransData &td,
                                          bPoseChannel &pchan);
/** Each update: solve aims and store the pose into the sequence. */
void pose_sequence_transform_recalc(TransInfo *t, TransDataContainer *tc);
/** When the transform ends: restore the saved frame if it was canceled. */
void pose_sequence_transform_end(TransInfo *t, TransDataContainer *tc);

}  // namespace blender::ed::transform
