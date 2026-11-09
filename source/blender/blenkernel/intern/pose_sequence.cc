/* SPDX-FileCopyrightText: 2026 Ephemeral Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup bke
 */

#include <optional>

#include "MEM_guardedalloc.h"

#include "DNA_pose_sequence_types.h"

#include "BLI_utildefines.h"

#include "BKE_idtype.hh"
#include "BKE_lib_id.hh"
#include "BKE_pose_sequence.hh"

#include "BLT_translation.hh"

#include "BLO_read_write.hh"

namespace blender {

static void pose_sequence_init_data(ID *id)
{
  PoseSequence *seq = id_cast<PoseSequence *>(id);
  INIT_DEFAULT_STRUCT_AFTER(seq, id);
}

static void pose_sequence_copy_data(Main * /*bmain*/,
                                    std::optional<Library *> /*owner_library*/,
                                    ID *id_dst,
                                    const ID *id_src,
                                    const int /*flag*/)
{
  PoseSequence *seq_dst = id_cast<PoseSequence *>(id_dst);
  const PoseSequence *seq_src = id_cast<const PoseSequence *>(id_src);
  seq_dst->bones = MEM_dupalloc(seq_src->bones);
  seq_dst->frame_states = MEM_dupalloc(seq_src->frame_states);
  seq_dst->transforms = MEM_dupalloc(seq_src->transforms);
}

static void pose_sequence_free_data(ID *id)
{
  PoseSequence *seq = id_cast<PoseSequence *>(id);
  MEM_SAFE_DELETE(seq->bones);
  MEM_SAFE_DELETE(seq->frame_states);
  MEM_SAFE_DELETE(seq->transforms);
}

static void pose_sequence_blend_write(BlendWriter *writer, ID *id, const void *id_address)
{
  PoseSequence *seq = id_cast<PoseSequence *>(id);

  writer->write_id_struct(id_address, seq);
  BKE_id_blend_write(writer, &seq->id);

  writer->write_struct_array(seq->bone_count, seq->bones);
  writer->write_uint8_array(seq->frame_count, seq->frame_states);
  writer->write_struct_array(int64_t(seq->frame_count) * seq->bone_count, seq->transforms);
}

static void pose_sequence_blend_read_data(BlendDataReader *reader, ID *id)
{
  PoseSequence *seq = id_cast<PoseSequence *>(id);

  const bool ok = BLO_read_array(reader, &seq->bones, seq->bone_count) &&
                  BLO_read_array(reader, &seq->frame_states, seq->frame_count) &&
                  BLO_read_array(reader, &seq->transforms, int64_t(seq->frame_count) * seq->bone_count);
  if (!ok) {
    /* Sizes don't match the stored arrays: drop the poses rather than read out of bounds. */
    pose_sequence_free_data(id);
    seq->bone_count = 0;
    seq->frame_count = 0;
  }
}

IDTypeInfo IDType_ID_PS = {
    .id_code = PoseSequence::id_type,
    .id_filter = FILTER_ID_PS,
    .dependencies_id_types = 0,
    .main_listbase_index = INDEX_ID_PS,
    .struct_size = sizeof(PoseSequence),
    .name = "PoseSequence",
    .name_plural = N_("pose_sequences"),
    .translation_context = BLT_I18NCONTEXT_ID_POSESEQUENCE,
    .flags = IDTYPE_FLAGS_NO_ANIMDATA,
    .asset_type_info = nullptr,

    .init_data = pose_sequence_init_data,
    .copy_data = pose_sequence_copy_data,
    .free_data = pose_sequence_free_data,
    .make_local = nullptr,
    .foreach_id = nullptr,
    .foreach_cache = nullptr,
    .foreach_path = nullptr,
    .foreach_working_space_color = nullptr,
    .owner_pointer_get = nullptr,

    .blend_write = pose_sequence_blend_write,
    .blend_read_data = pose_sequence_blend_read_data,
    .blend_read_after_liblink = nullptr,

    .blend_read_undo_preserve = nullptr,

    .lib_override_apply_post = nullptr,
};

PoseSequence *BKE_pose_sequence_add(Main *bmain, const char *name)
{
  return BKE_id_new<PoseSequence>(bmain, name);
}

}  // namespace blender
