/* SPDX-FileCopyrightText: 2026 Ephemeral Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup RNA
 */

#include <cfloat>
#include <cstdlib>

#include "DNA_scene_types.h"

#include "RNA_define.hh"

#include "rna_internal.hh"

#include "WM_types.hh"

#ifdef RNA_RUNTIME

#  include "DNA_armature_types.h"
#  include "DNA_object_types.h"
#  include "DNA_pose_sequence_types.h"

#  include "BLI_math_vector.h"

#  include "BKE_library.hh"
#  include "BKE_report.hh"

#  include "DEG_depsgraph.hh"

#  include "WM_api.hh"

#  include "EPH_pose_sequence.hh"

namespace blender {

static void rna_PoseSequence_changed(PoseSequence *seq)
{
  DEG_id_tag_update(&seq->id, ID_RECALC_PARAMETERS);
  WM_main_add_notifier(NC_OBJECT | ND_POSE, nullptr);
}

static void rna_PoseSequence_init(PoseSequence *seq,
                                  ReportList *reports,
                                  Object *armature_object,
                                  const int frame_start,
                                  const int frame_count)
{
  if (!ID_IS_EDITABLE(seq)) {
    BKE_report(reports, RPT_ERROR, "Pose sequence is not editable");
    return;
  }
  if (armature_object->type != OB_ARMATURE) {
    BKE_report(reports, RPT_ERROR, "Object is not an armature");
    return;
  }
  ephemeral::pose_sequence_init(
      *seq, *id_cast<const bArmature *>(armature_object->data), frame_start, frame_count);
  rna_PoseSequence_changed(seq);
}

static void rna_PoseSequence_set_bone_transform(PoseSequence *seq,
                                                ReportList *reports,
                                                const int frame,
                                                const char *bone,
                                                const float location[3],
                                                const float rotation[4],
                                                const float scale[3])
{
  if (!ID_IS_EDITABLE(seq)) {
    BKE_report(reports, RPT_ERROR, "Pose sequence is not editable");
    return;
  }
  PoseSequenceTransform transform;
  copy_v3_v3(transform.location, location);
  copy_v4_v4(transform.rotation, rotation);
  copy_v3_v3(transform.scale, scale);
  if (!ephemeral::pose_sequence_store_bone(*seq, frame, bone, transform)) {
    BKE_reportf(reports,
                RPT_ERROR,
                "Bone '%s' at frame %d is not in pose sequence '%s'",
                bone,
                frame,
                seq->id.name + 2);
    return;
  }
  rna_PoseSequence_changed(seq);
}

}  // namespace blender

#else

namespace blender {

static void rna_def_pose_sequence(BlenderRNA *brna)
{
  StructRNA *srna;
  PropertyRNA *prop;
  FunctionRNA *func;
  PropertyRNA *parm;

  srna = RNA_def_struct(brna, "PoseSequence", "ID");
  RNA_def_struct_ui_text(
      srna, "Pose Sequence", "Interpolationless animation stored as an explicit pose per frame");
  RNA_def_struct_ui_icon(srna, ICON_ARMATURE_DATA);

  prop = RNA_def_property(srna, "frame_start", PROP_INT, PROP_TIME);
  RNA_def_property_clear_flag(prop, PROP_EDITABLE);
  RNA_def_property_ui_text(prop, "Start Frame", "Scene frame of the first stored pose");

  prop = RNA_def_property(srna, "frame_count", PROP_INT, PROP_NONE);
  RNA_def_property_clear_flag(prop, PROP_EDITABLE);
  RNA_def_property_ui_text(prop, "Frame Count", "Number of frames the sequence covers");

  prop = RNA_def_property(srna, "bone_count", PROP_INT, PROP_NONE);
  RNA_def_property_clear_flag(prop, PROP_EDITABLE);
  RNA_def_property_ui_text(prop, "Bone Count", "Number of bones each pose covers");

  func = RNA_def_function(srna, "init", "rna_PoseSequence_init");
  RNA_def_function_ui_description(
      func, "Cover every bone of an armature over a frame range, discarding any stored poses");
  RNA_def_function_flag(func, FUNC_USE_REPORTS);
  parm = RNA_def_pointer(func, "armature_object", "Object", "", "Armature object");
  RNA_def_parameter_flags(parm, PROP_NEVER_NULL, PARM_REQUIRED);
  parm = RNA_def_int(
      func, "frame_start", 1, -MAXFRAME, MAXFRAME, "", "First frame", -MAXFRAME, MAXFRAME);
  RNA_def_parameter_flags(parm, PropertyFlag(0), PARM_REQUIRED);
  parm = RNA_def_int(func, "frame_count", 1, 1, MAXFRAME, "", "Number of frames", 1, MAXFRAME);
  RNA_def_parameter_flags(parm, PropertyFlag(0), PARM_REQUIRED);

  func = RNA_def_function(srna, "set_bone_transform", "rna_PoseSequence_set_bone_transform");
  RNA_def_function_ui_description(
      func, "Store one bone's transform on a frame, marking that frame as authored");
  RNA_def_function_flag(func, FUNC_USE_REPORTS);
  parm = RNA_def_int(
      func, "frame", 1, -MAXFRAME, MAXFRAME, "", "Scene frame", -MAXFRAME, MAXFRAME);
  RNA_def_parameter_flags(parm, PropertyFlag(0), PARM_REQUIRED);
  parm = RNA_def_string(func, "bone", nullptr, 0, "", "Bone name");
  RNA_def_parameter_flags(parm, PropertyFlag(0), PARM_REQUIRED);
  parm = RNA_def_float_vector(
      func, "location", 3, nullptr, -FLT_MAX, FLT_MAX, "", "Location", -FLT_MAX, FLT_MAX);
  RNA_def_parameter_flags(parm, PropertyFlag(0), PARM_REQUIRED);
  parm = RNA_def_float_vector(func,
                              "rotation",
                              4,
                              nullptr,
                              -FLT_MAX,
                              FLT_MAX,
                              "",
                              "Rotation quaternion (w, x, y, z)",
                              -FLT_MAX,
                              FLT_MAX);
  RNA_def_parameter_flags(parm, PropertyFlag(0), PARM_REQUIRED);
  parm = RNA_def_float_vector(
      func, "scale", 3, nullptr, -FLT_MAX, FLT_MAX, "", "Scale", -FLT_MAX, FLT_MAX);
  RNA_def_parameter_flags(parm, PropertyFlag(0), PARM_REQUIRED);
}

void RNA_def_pose_sequence(BlenderRNA *brna)
{
  rna_def_pose_sequence(brna);
}

}  // namespace blender

#endif
