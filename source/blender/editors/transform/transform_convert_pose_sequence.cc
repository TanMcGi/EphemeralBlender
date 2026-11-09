/* SPDX-FileCopyrightText: 2026 Ephemeral Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup edtransform
 */

#include <algorithm>
#include <memory>
#include <optional>

#include "DNA_action_types.h"
#include "DNA_armature_types.h"
#include "DNA_object_types.h"
#include "DNA_pose_sequence_types.h"
#include "DNA_scene_types.h"

#include "BLI_array.hh"
#include "BLI_map.hh"
#include "BLI_math_matrix.h"
#include "BLI_math_matrix_types.hh"
#include "BLI_math_vector.h"

#include "BKE_armature.hh"
#include "BKE_report.hh"

#include "DEG_depsgraph.hh"

#include "EPH_pose_sequence.hh"
#include "EPH_solver.hh"

#include "transform.hh"
#include "transform_convert_pose_sequence.hh"

namespace blender::ed::transform {

namespace {

/** A grabbed bone's head, which its parent is aimed at. */
struct AimControl {
  bPoseChannel *pchan = nullptr;
  /** What the transform moves (armature space). */
  float3 position;
  float3 initial_head;
  float4x4 initial_parent_pose_mat;
};

/** The edit of one armature's Pose Sequence during one transform. */
struct SequenceEdit {
  PoseSequence *seq = nullptr;
  int frame = 0;
  /** Index of #frame in the sequence, -1 when it's outside. */
  int frame_index = -1;
  Array<PoseSequenceTransform> saved_transforms;
  uint8_t saved_state = POSE_SEQUENCE_FRAME_EMPTY;
  /** One per #TransData of the container; only grabbed bones with a parent are used. */
  Array<AimControl> controls;
};

/** Edits in progress, from data creation until the transform ends. */
Map<const TransDataContainer *, std::unique_ptr<SequenceEdit>> &edits()
{
  static Map<const TransDataContainer *, std::unique_ptr<SequenceEdit>> map;
  return map;
}

SequenceEdit *find_edit(const TransDataContainer *tc)
{
  std::unique_ptr<SequenceEdit> *edit = edits().lookup_ptr(tc);
  return edit ? edit->get() : nullptr;
}

}  // namespace

void pose_sequence_transform_begin(TransInfo *t, TransDataContainer *tc)
{
  Object *ob = tc->poseobj;
  PoseSequence *seq = ob->pose_sequence;
  if (seq == nullptr || ELEM(t->mode, TFM_DUMMY, TFM_BONESIZE, TFM_BONE_ENVELOPE_DIST)) {
    return;
  }

  auto edit = std::make_unique<SequenceEdit>();
  edit->seq = seq;
  edit->frame = t->scene->r.cfra;
  const int index = edit->frame - seq->frame_start;
  if (index >= 0 && index < seq->frame_count) {
    edit->frame_index = index;
    edit->saved_transforms = Span(&seq->transforms[int64_t(index) * seq->bone_count],
                                  seq->bone_count);
    edit->saved_state = seq->frame_states[index];
  }
  else {
    BKE_report(t->reports,
               RPT_WARNING,
               "Current frame is outside the pose sequence, the change will not be stored");
  }
  edit->controls.reinitialize(tc->data_len);

  /* What's on screen comes from the sequence, not the channels: start from it. */
  ephemeral::pose_sequence_apply(*seq, edit->frame, *ob->pose);

  edits().add_overwrite(tc, std::move(edit));
}

void pose_sequence_transform_control_init(TransInfo *t,
                                          TransDataContainer *tc,
                                          TransData &td,
                                          bPoseChannel &pchan)
{
  if (t->mode != TFM_TRANSLATION || pchan.parent == nullptr) {
    return;
  }
  SequenceEdit *edit = find_edit(tc);
  if (edit == nullptr) {
    return;
  }

  AimControl &control = edit->controls[&td - tc->data];
  control.pchan = &pchan;
  control.initial_head = float3(pchan.pose_head);
  control.position = control.initial_head;
  control.initial_parent_pose_mat = float4x4(pchan.parent->pose_mat);

  /* Like a target-less IK grab: the control moves freely in object space. */
  td.loc = &control.position.x;
  copy_v3_v3(td.iloc, td.loc);
  copy_m3_m4(td.mtx, tc->poseobj->object_to_world().ptr());
  pseudoinverse_m3_m3(td.smtx, td.mtx, PSEUDOINVERSE_EPSILON);
}

void pose_sequence_transform_recalc(TransInfo * /*t*/, TransDataContainer *tc)
{
  SequenceEdit *edit = find_edit(tc);
  if (edit == nullptr) {
    return;
  }
  Object *ob = tc->poseobj;

  for (const AimControl &control : edit->controls) {
    if (control.pchan == nullptr) {
      continue;
    }
    bPoseChannel *parent = control.pchan->parent;
    const float4x4 aimed = ephemeral::aim_bone(
        control.initial_parent_pose_mat, control.initial_head, control.position);

    /* Back to the parent's own channel values; only its rotation changes. */
    float local[4][4];
    BKE_armature_mat_pose_to_bone({parent, parent->bone_get(*ob)}, aimed.ptr(), local);
    float scale[3];
    normalize_m4_ex(local, scale);
    float rotation[3][3];
    copy_m3_m4(rotation, local);
    BKE_pchan_mat3_to_rot(parent, rotation, false);
  }

  if (edit->frame_index >= 0) {
    ephemeral::pose_sequence_store_pose(*edit->seq, edit->frame, *ob->pose);
    DEG_id_tag_update(&edit->seq->id, ID_RECALC_PARAMETERS);
  }
}

void pose_sequence_transform_end(TransInfo *t, TransDataContainer *tc)
{
  std::optional<std::unique_ptr<SequenceEdit>> popped = edits().pop_try(tc);
  if (!popped) {
    return;
  }
  SequenceEdit &edit = **popped;
  if (t->state != TRANS_CANCEL || edit.frame_index < 0) {
    return;
  }

  PoseSequence &seq = *edit.seq;
  std::copy(edit.saved_transforms.begin(),
            edit.saved_transforms.end(),
            &seq.transforms[int64_t(edit.frame_index) * seq.bone_count]);
  seq.frame_states[edit.frame_index] = edit.saved_state;
  ephemeral::pose_sequence_apply(seq, edit.frame, *tc->poseobj->pose);
  DEG_id_tag_update(&seq.id, ID_RECALC_PARAMETERS);
}

}  // namespace blender::ed::transform
