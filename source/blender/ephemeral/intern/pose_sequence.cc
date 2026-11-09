/* SPDX-FileCopyrightText: 2026 Ephemeral Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup ephemeral
 */

#include <algorithm>

#include "MEM_guardedalloc.h"

#include "BLI_listbase_iterator.hh"
#include "BLI_math_rotation.h"
#include "BLI_math_vector.h"
#include "BLI_string.h"
#include "BLI_vector.hh"

#include "DNA_action_types.h"
#include "DNA_armature_types.h"
#include "DNA_pose_sequence_types.h"

#include "EPH_pose_sequence.hh"

namespace blender::ephemeral {

static void collect_bones(const ListBaseT<Bone> &bones, Vector<const Bone *> &r_bones)
{
  for (const Bone &bone : bones) {
    r_bones.append(&bone);
    collect_bones(bone.childbase, r_bones);
  }
}

void pose_sequence_init(PoseSequence &seq,
                        const bArmature &armature,
                        const int frame_start,
                        const int frame_count)
{
  Vector<const Bone *> bones;
  collect_bones(armature.bonebase, bones);

  MEM_SAFE_DELETE(seq.bones);
  MEM_SAFE_DELETE(seq.frame_states);
  MEM_SAFE_DELETE(seq.transforms);

  seq.frame_start = frame_start;
  seq.frame_count = frame_count;
  seq.bone_count = int(bones.size());
  seq.bones = MEM_new_array<PoseSequenceBone>(bones.size(), __func__);
  for (const int i : bones.index_range()) {
    STRNCPY(seq.bones[i].name, bones[i]->name);
  }
  seq.frame_states = MEM_new_array_zeroed<uint8_t>(size_t(frame_count), __func__);
  seq.transforms = MEM_new_array<PoseSequenceTransform>(size_t(frame_count) * bones.size(),
                                                        __func__);
}

int pose_sequence_frame_to_show(const PoseSequence &seq, const int frame)
{
  const int last = seq.frame_count - 1;
  for (int index = std::min(frame - seq.frame_start, last); index >= 0; index--) {
    if (seq.frame_states[index] != POSE_SEQUENCE_FRAME_EMPTY) {
      return index;
    }
  }
  return -1;
}

bool pose_sequence_store_bone(PoseSequence &seq,
                              const int frame,
                              const StringRefNull bone_name,
                              const PoseSequenceTransform &transform)
{
  const int index = frame - seq.frame_start;
  if (index < 0 || index >= seq.frame_count) {
    return false;
  }
  int bone = 0;
  while (bone < seq.bone_count && bone_name != seq.bones[bone].name) {
    bone++;
  }
  if (bone == seq.bone_count) {
    return false;
  }

  PoseSequenceTransform *frame_transforms = &seq.transforms[int64_t(index) * seq.bone_count];
  if (seq.frame_states[index] == POSE_SEQUENCE_FRAME_EMPTY) {
    const int shown = pose_sequence_frame_to_show(seq, frame);
    if (shown >= 0) {
      std::copy_n(&seq.transforms[int64_t(shown) * seq.bone_count], seq.bone_count,
                  frame_transforms);
    }
  }
  frame_transforms[bone] = transform;
  seq.frame_states[index] = POSE_SEQUENCE_FRAME_AUTHORED;
  return true;
}

static void channel_rotation_to_quat(const bPoseChannel &pchan, float r_quat[4])
{
  switch (pchan.rotmode) {
    case ROT_MODE_QUAT:
      normalize_qt_qt(r_quat, pchan.quat);
      break;
    case ROT_MODE_AXISANGLE:
      axis_angle_to_quat(r_quat, pchan.rotAxis, pchan.rotAngle);
      break;
    default:
      eulO_to_quat(r_quat, pchan.eul, pchan.rotmode);
      break;
  }
}

bool pose_sequence_store_pose(PoseSequence &seq, const int frame, const bPose &pose)
{
  const int index = frame - seq.frame_start;
  if (index < 0 || index >= seq.frame_count) {
    return false;
  }
  PoseSequenceTransform *frame_transforms = &seq.transforms[int64_t(index) * seq.bone_count];
  for (const bPoseChannel &pchan : pose.chanbase) {
    for (int bone = 0; bone < seq.bone_count; bone++) {
      if (!STREQ(seq.bones[bone].name, pchan.name)) {
        continue;
      }
      PoseSequenceTransform &transform = frame_transforms[bone];
      copy_v3_v3(transform.location, pchan.loc);
      channel_rotation_to_quat(pchan, transform.rotation);
      copy_v3_v3(transform.scale, pchan.scale);
      break;
    }
  }
  seq.frame_states[index] = POSE_SEQUENCE_FRAME_AUTHORED;
  return true;
}

static void apply_rotation(const float quat[4], bPoseChannel &pchan)
{
  switch (pchan.rotmode) {
    case ROT_MODE_QUAT:
      copy_qt_qt(pchan.quat, quat);
      break;
    case ROT_MODE_AXISANGLE:
      quat_to_axis_angle(pchan.rotAxis, &pchan.rotAngle, quat);
      break;
    default:
      quat_to_eulO(pchan.eul, pchan.rotmode, quat);
      break;
  }
}

void pose_sequence_apply(const PoseSequence &seq, const int frame, bPose &pose)
{
  const int index = pose_sequence_frame_to_show(seq, frame);
  if (index < 0) {
    return;
  }
  const PoseSequenceTransform *frame_transforms = &seq.transforms[int64_t(index) *
                                                                   seq.bone_count];
  for (bPoseChannel &pchan : pose.chanbase) {
    for (int bone = 0; bone < seq.bone_count; bone++) {
      if (!STREQ(seq.bones[bone].name, pchan.name)) {
        continue;
      }
      const PoseSequenceTransform &transform = frame_transforms[bone];
      copy_v3_v3(pchan.loc, transform.location);
      apply_rotation(transform.rotation, pchan);
      copy_v3_v3(pchan.scale, transform.scale);
      break;
    }
  }
}

}  // namespace blender::ephemeral
